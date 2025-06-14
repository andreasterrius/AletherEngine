module;

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

export module sdf_model_packed;
import texture;
import transform;
import shader;
import file_system;
import sdf_model;
import bounding_box;

using afs = ale::FileSystem;
using namespace ale::data;
using namespace std;
using namespace glm;

export namespace ale::graphics::sdf {

constexpr int ATLAS_WIDTH = 4096;
constexpr int ATLAS_HEIGHT = 4096;
constexpr int OBJECTS_MAX_SIZE = 2000;
constexpr int SINGLE_TEXTURE_SIZE_Y = 64;

class SdfModelPacked {
public:
  struct GPUObject {
    glm::mat4 model_mat;
    glm::mat4 inv_model_mat;
    glm::vec4 inner_bbmin;
    glm::vec4 inner_bbmax;
    glm::vec4 outer_bbmin;
    glm::vec4 outer_bbmax;
    int atlas_index;
    int atlas_count;
    int _a = 0;
    int _b = 0;
  };

  struct Meta {
    glm::ivec3 size;
    BoundingBox inner_bb;
    BoundingBox outer_bb;
    int atlas_index; // index of texture_atlas
    int atlas_count; // index of texture number in atlas
  };

private:
  std::vector<Texture> texture_atlas;
  std::vector<Meta> offsets;
  bool debug_mode;
  int ssbo;

  std::vector<unsigned int>
  pack_sdf_models(std::vector<SdfModel *> sdf_models) {
    auto flat_data = vector(ATLAS_WIDTH * ATLAS_HEIGHT, 0.0f);
    auto packed_count = 0; // how many texture is packed inside an atlas
    auto entries = vector<unsigned int>{};

    for (auto &it: sdf_models) {
      auto sdf_data = it->texture3D->retrieve_data_from_gpu();
      Texture3D::Meta meta = it->texture3D->meta;

      ivec3 size = ivec3(meta.width, meta.height, meta.depth);
      ivec3 size2d = ivec3(64, 64, 64);
      for (int i = 0; i < sdf_data.size(); ++i) {
        unsigned int z = i / (size.x * size.y);
        unsigned int y = (i % (size.x * size.y) / size.x);
        unsigned int x = i % size.x;

        unsigned int flat_index =
            x + (y * size2d.x * size2d.z) + (z * size2d.x);

        // stack it
        flat_index = packed_count * size2d.x * size2d.z * size2d.y + flat_index;

        // finish the remapping
        // cout << x << " " << y << " " << z << " | " << flat_index << "\n";
        flat_data[flat_index] = sdf_data[i];
      }

      entries.push_back(offsets.size());
      offsets.push_back(SdfModelPacked::Meta{
          .size = size,
          .inner_bb = it->bb,
          .outer_bb = it->outerBB,
          .atlas_index = (int) texture_atlas.size(),
          .atlas_count = packed_count,
      });
      packed_count += 1;

      if (&it == &sdf_models.back() || packed_count >= 64) {
        // we has filled in this texture, push and create a new one
        texture_atlas.emplace_back(
            Texture::Meta{
                .width = ATLAS_WIDTH,
                .height = ATLAS_HEIGHT,
                .internal_format = GL_R32F,
                .input_format = GL_RED,
                .input_type = GL_FLOAT,
                .min_filter = GL_LINEAR,
                .max_filter = GL_LINEAR,
            },
            flat_data);
        packed_count = 0;
      }
    }

    unsigned int ssbo = 0;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(
        GL_SHADER_STORAGE_BUFFER,
        (sizeof(unsigned int) * 4 + sizeof(GPUObject) * OBJECTS_MAX_SIZE),
        nullptr, GL_STATIC_DRAW);
    this->ssbo = ssbo;

    if (debug_mode) {
      for (int i = 0; i < texture_atlas.size(); ++i) {
        texture_atlas[i].dump_data_to_file(
            afs::root("debug/atlas_" + to_string(i) + ".txt"));
      }
    }

    return entries;
  }

public:
  // data will be copied, just need to have a temporary reference to sdf models
  SdfModelPacked(std::vector<SdfModel *> sdf_models, bool debug_mode = false) :
      debug_mode(debug_mode) {
    pack_sdf_models(sdf_models);
  }

  SdfModelPacked(SdfModelPacked &other) = delete;
  SdfModelPacked &operator=(SdfModelPacked &other) = delete;

  SdfModelPacked(SdfModelPacked &&other) :
      texture_atlas(std::move(other.texture_atlas)),
      offsets(std::move(other.offsets)),
      debug_mode(other.debug_mode) {}
  SdfModelPacked &operator=(SdfModelPacked &&other) {
    if (this != &other) {
      std::swap(this->texture_atlas, other.texture_atlas);
      std::swap(this->offsets, other.offsets);
      this->debug_mode = other.debug_mode;
    }

    return *this;
  }

  void bind_to_shader(
      Shader &shader,
      // transform -> shadow mesh index
      std::vector<std::pair<Transform, std::vector<unsigned int>>> &entries,
      int atlas_start_index) {
    glDisable(GL_CULL_FACE);

    auto details = vector<GPUObject>();
    // TODO: no need to do this every frame, only when a change occur
    // TODO: shader supports 1 MESH = 1 SDF, not 1 MODEL = 1 SDF
    for (auto [transform, shadow_indices]: entries) {
      for (auto shadow_index: shadow_indices) {
        auto &p = offsets[shadow_index];
        mat4 model = transform.get_model_matrix();
        details.push_back(GPUObject{
            .model_mat = model,
            .inv_model_mat = inverse(model),
            .inner_bbmin = vec4(p.inner_bb.min, 0.0),
            .inner_bbmax = vec4(p.inner_bb.max, 0.0),
            .outer_bbmin = vec4(p.outer_bb.min, 0.0),
            .outer_bbmax = vec4(p.outer_bb.max, 0.0),
            .atlas_index = p.atlas_index,
            .atlas_count = p.atlas_count,
        });
      }
    }
    int details_size = details.size();

    // ssbo for packed sdf
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, (sizeof(unsigned int) * 4),
                    &details_size); // pass size
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, (sizeof(unsigned int) * 4),
                    (details.size() * sizeof(GPUObject)), details.data());

    // bind ssbo
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

    shader.use();
    shader.setInt("atlasStartIndex", atlas_start_index);
    shader.setInt("atlasSize", this->texture_atlas.size());
    if (this->texture_atlas.empty()) {
      return;
    }

    int texture_units[6] = {0};
    for (int i = 0; i < this->texture_atlas.size(); ++i) {
      texture_units[i] = i + atlas_start_index;
    }
    GLint location = glGetUniformLocation(shader.ID, "atlas");
    glUniform1iv(location, this->texture_atlas.size(), texture_units);

    for (int i = 0; i < this->texture_atlas.size(); ++i) {
      glActiveTexture(GL_TEXTURE0 + atlas_start_index + i);
      glBindTexture(GL_TEXTURE_2D, texture_atlas[i].id);
    }
  }

  unsigned int add(SdfModel &sdf_model) {
    auto [latest_index, latest_count] =
        offsets.empty() ? make_pair(0, 0)
                        : make_pair(offsets.back().atlas_index,
                                    offsets.back().atlas_count + 1);

    // 1 texture can only hold 64 models
    if (latest_count >= 64) {
      latest_index += 1;
      latest_count = 0;
    }

    // if count = 0, then we need to create a new texture
    if (latest_count == 0) {
      auto empty = vector<float>();
      texture_atlas.emplace_back(
          Texture::Meta{
              .width = ATLAS_WIDTH,
              .height = ATLAS_HEIGHT,
              .internal_format = GL_R32F,
              .input_format = GL_RED,
              .input_type = GL_FLOAT,
              .min_filter = GL_LINEAR,
              .max_filter = GL_LINEAR,
          },
          empty);
    }

    auto sdf_data = sdf_model.texture3D->retrieve_data_from_gpu();
    auto meta = sdf_model.texture3D->meta;

    auto flat_data = vector(ATLAS_WIDTH * SINGLE_TEXTURE_SIZE_Y, 0.0f);
    auto size = ivec3(meta.width, meta.height, meta.depth);
    auto size2d = ivec3(64, 64, 64);
    for (int i = 0; i < sdf_data.size(); ++i) {
      unsigned int z = i / (size.x * size.y);
      unsigned int y = (i % (size.x * size.y) / size.x);
      unsigned int x = i % size.x;

      unsigned int flat_index = x + (y * size2d.x * size2d.z) + (z * size2d.x);

      // finish the remapping
      // cout << x << " " << y << " " << z << " | " << flat_index << "\n";
      flat_data[flat_index] = sdf_data[i];
    }

    texture_atlas.back().partial_replace_data_f32(
        0, latest_count * SINGLE_TEXTURE_SIZE_Y, ATLAS_WIDTH,
        SINGLE_TEXTURE_SIZE_Y, flat_data);

    offsets.push_back(Meta{
        .size = size,
        .inner_bb = sdf_model.bb,
        .outer_bb = sdf_model.outerBB,
        .atlas_index = latest_index,
        .atlas_count = latest_count,
    });

    return offsets.size() - 1;
  }

  std::vector<Meta> &get_offsets() { return this->offsets; }
  std::vector<Texture> &get_texture_atlas() { return this->texture_atlas; }
};

} // namespace ale::graphics::sdf
