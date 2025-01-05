#ifndef BASIC_RENDERER_H
#define BASIC_RENDERER_H

#include <entt/entt.hpp>
#include <string>

#include "../camera.h"
#include "../data/shader.h"
#include "../sdf_generator_gpu.h"
#include "../sdf_model_packed.h"

namespace ale {

class BasicRendererException final : public runtime_error {
public:
  explicit BasicRendererException(const string &msg) : runtime_error(msg) {}
};

struct Light {
  vec3 position;
};

class BasicRenderer {
private:
  Shader basicShader;

  bool debug_mode = true;

public:
  BasicRenderer();

  void render(Camera &camera, vector<Light> &lights, entt::registry &world);
};
} // namespace ale

#endif // BASIC_RENDERER_H
