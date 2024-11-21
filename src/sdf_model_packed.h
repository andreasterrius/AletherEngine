#ifndef ALETHERENGINE_SDF_MODEL_PACKED_H
#define ALETHERENGINE_SDF_MODEL_PACKED_H

#include <glm/glm.hpp>
#include <vector>

#include "data/shader.h"
#include "sdf_model.h"
#include "texture.h"

using namespace glm;
using namespace std;

namespace ale {

constexpr int ATLAS_WIDTH = 4096;
constexpr int ATLAS_HEIGHT = 4096;
constexpr int SINGLE_TEXTURE_SIZE_Y = 64;

class SdfModelPacked {
 private:
  struct SdfModelPackedMeta {
    ivec3 size;
    BoundingBox inner_bb;
    BoundingBox outer_bb;
    int atlas_index;  // index of texture_atlas
    int atlas_count;  // index of texture number in atlas
  };

  vector<Texture> texture_atlas;
  vector<SdfModelPackedMeta> offsets;
  bool debug_mode;

  void pack_sdf_models(vector<SdfModel *> sdf_models);

 public:
  // data will be copied, just need to have a temporary reference to sdf models
  SdfModelPacked(vector<SdfModel *> sdf_models, bool debug_mode = false);

  SdfModelPacked(SdfModelPacked &other) = delete;
  SdfModelPacked &operator=(SdfModelPacked &other) = delete;

  SdfModelPacked(SdfModelPacked &&other);
  SdfModelPacked &operator=(SdfModelPacked &&other);

  void bind_to_shader(Shader &shader);
};

}  // namespace ale

#endif  // ALETHERENGINE_SDF_MODEL_PACKED_H
