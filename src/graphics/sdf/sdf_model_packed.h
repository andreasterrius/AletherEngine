#ifndef ALETHERENGINE_SDF_MODEL_PACKED_H
#define ALETHERENGINE_SDF_MODEL_PACKED_H

#include <glm/glm.hpp>
#include <vector>

#include "src/graphics/sdf/sdf_model.h"
#include "src/graphics/shader.h"
#include "src/graphics/texture.h"

namespace ale {

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

  std::vector<unsigned int> pack_sdf_models(std::vector<SdfModel *> sdf_models);

public:
  // data will be copied, just need to have a temporary reference to sdf models
  SdfModelPacked(std::vector<SdfModel *> sdf_models, bool debug_mode = false);

  SdfModelPacked(SdfModelPacked &other) = delete;
  SdfModelPacked &operator=(SdfModelPacked &other) = delete;

  SdfModelPacked(SdfModelPacked &&other);
  SdfModelPacked &operator=(SdfModelPacked &&other);

  void bind_to_shader(
      Shader &shader,
      // transform -> shadow mesh index
      std::vector<std::pair<Transform, std::vector<unsigned int>>> &entries);

  unsigned int add(SdfModel &sdf_model);

  std::vector<Meta> &get_offsets();
  std::vector<Texture> &get_texture_atlas();
};

} // namespace ale

#endif // ALETHERENGINE_SDF_MODEL_PACKED_H
