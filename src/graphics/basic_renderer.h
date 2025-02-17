#ifndef BASIC_RENDERER_H
#define BASIC_RENDERER_H

#include "camera.h"
#include "sdf/sdf_model_packed.h"
#include "shader.h"
#include "src/data/serde/glm.h"
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>
#include <string>

namespace ale {

class BasicRendererException final : public std::runtime_error {
public:
  explicit BasicRendererException(const std::string &msg)
      : runtime_error(msg) {}
};

struct Light {
  glm::vec3 color = glm::vec3(1.0f);
  float radius = 1.0f;
  glm::vec3 attenuation = glm::vec3(1.0f, 0.09f, 0.032f);
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
