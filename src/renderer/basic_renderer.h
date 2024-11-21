#ifndef BASIC_RENDERER_H
#define BASIC_RENDERER_H

#include <string>

#include "../camera.h"
#include "../components/renderable.h"
#include "../data/shader.h"
#include "../sdf_generator_gpu.h"
#include "../sdf_model_packed.h"

namespace ale {

struct Light {};

class BasicRenderer {
 private:
  Shader basic_shader;

  bool debug_mode = true;

 public:
  BasicRenderer();

  void render(Camera &camera, vector<Light> &lights,
              SdfModelPacked &sdf_model_packed);
};
}  // namespace ale

#endif  // BASIC_RENDERER_H
