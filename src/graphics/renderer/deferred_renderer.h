#ifndef BASIC_RENDERER_H
#define BASIC_RENDERER_H

#include "src/data/serde/glm.h"
#include "src/graphics/camera.h"
#include "src/graphics/framebuffer.h"
#include "src/graphics/sdf/sdf_model_packed.h"
#include "src/graphics/shader.h"
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>
#include <string>

namespace ale {

class DeferredRendererException final : public std::runtime_error {
public:
  explicit DeferredRendererException(const std::string &msg)
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
  float specular_color = 1.0f;
  std::shared_ptr<Texture> diffuse_texture = nullptr;
  std::shared_ptr<Texture> specular_texture = nullptr;
  // std::shared_ptr<Texture> roughness_texture;
  // std::shared_ptr<Texture> metalness_texture;
  // std::shared_ptr<Texture> normal_texture;
  // std::shared_ptr<Texture> ao_texture;
};

class DeferredRenderer : public WindowEventListener {
private:
  Shader first_pass;
  Shader second_pass;
  Texture single_black_pixel_texture;

  Framebuffer deferred_framebuffer;
  TextureRenderer texture_renderer;
  WindowEventProducer *event_producer = nullptr;

public:
  DeferredRenderer(glm::ivec2 screen_size);
  ~DeferredRenderer();

  void add_listener(WindowEventProducer *event_producer);

  void render(Camera &camera, entt::registry &world);

  void set_texture_with_default(std::string name, int location,
                                const Texture *texture) const;

public:
  void mouse_button_callback(int button, int action, int mods) override;
  void cursor_pos_callback(double xpos, double ypos, double xoffset,
                           double yoffset) override;
  void framebuffer_size_callback(int width, int) override;
  void scroll_callback(double x_offset, double y_offset) override;
  void key_callback(int key, int scancode, int action, int mods) override;
};
} // namespace ale

#endif // BASIC_RENDERER_H
