#ifndef BASIC_RENDERER_H
#define BASIC_RENDERER_H

#include "camera.h"
#include "framebuffer.h"
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

struct BasicMaterial {
public:
  glm::vec3 diffuse_color = glm::vec3(1.0f);
  std::shared_ptr<Texture> diffuse_texture = nullptr;
  // std::shared_ptr<Texture> specular_texture;
  // std::shared_ptr<Texture> roughness_texture;
  // std::shared_ptr<Texture> metalness_texture;
  // std::shared_ptr<Texture> normal_texture;
  // std::shared_ptr<Texture> ao_texture;
};

class BasicRenderer {
private:
  Shader color_shader;
  Texture single_black_pixel_texture;

  // Framebuffer deferred_framebuffer;

  bool debug_mode = true;

public:
  BasicRenderer();

  void render(Camera &camera, entt::registry &world);

  void set_texture_with_default(std::string name, int location,
                                const Texture *texture) const;
};
} // namespace ale

#endif // BASIC_RENDERER_H
