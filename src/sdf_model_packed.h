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
constexpr int OBJECTS_MAX_SIZE = 2000;
constexpr int SINGLE_TEXTURE_SIZE_Y = 64;

class SdfModelPacked {
public:
  struct GPUObject {
    mat4 model_mat;
    mat4 inv_model_mat;
    vec4 inner_bbmin;
    vec4 inner_bbmax;
    vec4 outer_bbmin;
    vec4 outer_bbmax;
    int atlas_index;
    int atlas_count;
    int _a = 0;
    int _b = 0;
  };

  struct Meta {
    ivec3 size;
    BoundingBox inner_bb;
    BoundingBox outer_bb;
    int atlas_index; // index of texture_atlas
    int atlas_count; // index of texture number in atlas
  };

private:
  vector<Texture> texture_atlas;
  vector<Meta> offsets;
  bool debug_mode;
  int ssbo;

  vector<unsigned int> pack_sdf_models(vector<SdfModel *> sdf_models);

public:
  // data will be copied, just need to have a temporary reference to sdf models
  SdfModelPacked(vector<SdfModel *> sdf_models, bool debug_mode = false);

  SdfModelPacked(SdfModelPacked &other) = delete;
  SdfModelPacked &operator=(SdfModelPacked &other) = delete;

  SdfModelPacked(SdfModelPacked &&other);
  SdfModelPacked &operator=(SdfModelPacked &&other);

  void bind_to_shader(Shader &shader,
                      vector<pair<Transform, unsigned int>> &entries);

  vector<Meta> &get_offsets();
  vector<Texture> &get_texture_atlas();
};

} // namespace ale

#endif // ALETHERENGINE_SDF_MODEL_PACKED_H
