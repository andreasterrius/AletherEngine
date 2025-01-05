#include "sdf_model_packed.h"

#include "file_system.h"
using afs = ale::FileSystem;

vector<unsigned int>
ale::SdfModelPacked::pack_sdf_models(vector<SdfModel *> sdf_models) {
  auto flat_data = vector<float>(ATLAS_WIDTH * ATLAS_HEIGHT, 0.0f);
  auto packed_count = 0; // how many texture is packed inside an atlas
  auto entries = vector<unsigned int>{};

  for (auto &it : sdf_models) {
    auto sdf_data = it->texture3D->retrieve_data_from_gpu();
    Texture3D::Meta meta = it->texture3D->meta;

    ivec3 size = ivec3(meta.width, meta.height, meta.depth);
    ivec3 size2d = ivec3(64, 64, 64);
    for (int i = 0; i < sdf_data.size(); ++i) {
      unsigned int z = i / (size.x * size.y);
      unsigned int y = (i % (size.x * size.y) / size.x);
      unsigned int x = i % size.x;

      unsigned int flat_index = x + (y * size2d.x * size2d.z) + (z * size2d.x);

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
        .atlas_index = (int)texture_atlas.size(),
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
          afs::root("resources/sdfgen/atlas_" + to_string(i) + ".txt"));
    }
  }

  return entries;
}

ale::SdfModelPacked::SdfModelPacked(vector<SdfModel *> sdf_models,
                                    bool debug_mode)
    : debug_mode(debug_mode) {
  pack_sdf_models(sdf_models);
}

ale::SdfModelPacked::SdfModelPacked(SdfModelPacked &&other)
    : texture_atlas(std::move(other.texture_atlas)),
      offsets(std::move(other.offsets)), debug_mode(other.debug_mode) {}

ale::SdfModelPacked &ale::SdfModelPacked::operator=(SdfModelPacked &&other) {
  if (this != &other) {
    std::swap(this->texture_atlas, other.texture_atlas);
    std::swap(this->offsets, other.offsets);
    this->debug_mode = other.debug_mode;
  }

  return *this;
}

void ale::SdfModelPacked::bind_to_shader(
    Shader &shader, vector<pair<Transform, unsigned int>> &entries) {
  glDisable(GL_CULL_FACE);

  auto details = vector<GPUObject>();
  for (auto [transform, shadow_index] : entries) {
    auto &p = offsets[shadow_index];
    mat4 model = transform.getModelMatrix();
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
  int details_size = details.size();

  // ssbo for packed sdf
  glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, (sizeof(unsigned int) * 4),
                  &details_size); // pass size
  glBufferSubData(GL_SHADER_STORAGE_BUFFER, (sizeof(unsigned int) * 4),
                  (details.size() * sizeof(GPUObject)), details.data());

  // bind ssbo
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

  shader.use();
  shader.setInt("atlasSize", this->texture_atlas.size());
  if (this->texture_atlas.empty()) {
    return;
  }

  int texture_units[16] = {0};
  for (int i = 0; i < this->texture_atlas.size(); ++i) {
    texture_units[i] = i;
  }
  GLint location = glGetUniformLocation(shader.ID, "atlas");
  glUniform1iv(location, this->texture_atlas.size(), texture_units);

  for (int i = 0; i < this->texture_atlas.size(); ++i) {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, this->texture_atlas[i].id);
  }
}

vector<ale::SdfModelPacked::Meta> &ale::SdfModelPacked::get_offsets() {
  return this->offsets;
}

vector<Texture> &ale::SdfModelPacked::get_texture_atlas() {
  return this->texture_atlas;
}
