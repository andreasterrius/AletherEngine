#include "sdf_model_packed.h"

#include "file_system.h"

using afs = ale::FileSystem;

void ale::SdfModelPacked::pack_sdf_models(vector<SdfModel *> sdf_models) {
  auto flat_data = vector<float>(ATLAS_WIDTH * ATLAS_HEIGHT, 0.0f);
  int index = -1;

  for (auto &it : sdf_models) {
    auto sdf_data = it->texture3D->retrieve_data_from_gpu();
    Texture3D::Meta meta = it->texture3D->meta;

    index += 1;
    ivec3 size = ivec3(meta.width, meta.height, meta.depth);
    ivec3 size2d = ivec3(64, 64, 64);
    for (int i = 0; i < sdf_data.size(); ++i) {
      unsigned int z = i / (size.x * size.y);
      unsigned int y = (i % (size.x * size.y) / size.x);
      unsigned int x = i % size.x;

      unsigned int flat_index = x + (y * size2d.x * size2d.z) + (z * size2d.x);

      // stack it
      flat_index = index * size2d.x * size2d.z * size2d.y + flat_index;

      // finish the remapping
      cout << x << " " << y << " " << z << " | " << flat_index << "\n";
      flat_data[flat_index] = sdf_data[i];

      offsets.push_back(SdfModelPackedMeta{
          .size = size,
          .inner_bb = it->bb,
          .outer_bb = it->outerBB,
          .atlas_index = (int)texture_atlas.size() - 1,
          .atlas_count = index,
      });
    }

    if (&it == &sdf_models.back() || index >= 64) {
      // we has filled in this texture, push and create a new one
      texture_atlas.emplace_back(
          Texture(Texture::Meta{.width = 4096,
                                .height = 256,
                                .internal_format = GL_R32F,
                                .input_format = GL_RED,
                                .input_type = GL_FLOAT},
                  flat_data),
          index);
      index = -1;
    }
  }

  if (debug_mode) {
    for (int i = 0; i < texture_atlas.size(); ++i) {
      texture_atlas[i].dump_data_to_file(
          afs::root("resources/sdfgen/atlas_" + to_string(i) + ".txt"));
    }
  }
}

ale::SdfModelPacked::SdfModelPacked(vector<SdfModel *> sdf_models,
                                    bool debug_mode)
    : debug_mode(debug_mode) {
  pack_sdf_models(sdf_models);
}

ale::SdfModelPacked::SdfModelPacked(SdfModelPacked &&other)
    : texture_atlas(std::move(other.texture_atlas)),
      offsets(std::move(other.offsets)),
      debug_mode(other.debug_mode) {}

ale::SdfModelPacked &ale::SdfModelPacked::operator=(SdfModelPacked &&other) {
  if (this != &other) {
    std::swap(this->texture_atlas, other.texture_atlas);
    std::swap(this->offsets, other.offsets);
    this->debug_mode = other.debug_mode;
  }

  return *this;
}

void ale::SdfModelPacked::bind_to_shader(Shader &shader) {}
