#ifndef BASIC_RENDERER_H
#define BASIC_RENDERER_H

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>
#include <string>

#include "../camera.h"
#include "../data/serde/glm.h"
#include "../data/shader.h"
#include "../sdf_model_packed.h"

namespace ale {

class BasicRendererException final : public runtime_error {
public:
  explicit BasicRendererException(const string &msg) : runtime_error(msg) {}
};

struct Light {
  vec3 color = vec3(1.0f);
  float radius = 1.0f;
  vec3 attenuation = vec3(1.0f, 0.09f, 0.032f);
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Light, color, radius, attenuation);

class BasicRenderer {
private:
  Shader color_shader;

  bool debug_mode = true;

public:
  BasicRenderer();

  void render(Camera &camera, entt::registry &world);
};
} // namespace ale

#endif // BASIC_RENDERER_H
