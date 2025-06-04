#ifndef BASIC_RENDERER_H
#define BASIC_RENDERER_H

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include "src/data/serde/glm.h"
#include "src/graphics/camera.h"
#include "src/graphics/sdf/sdf_model_packed.h"
#include "src/graphics/shader.h"

namespace ale {
using namespace input_handling;

class BasicRendererException final : public std::runtime_error {
public:
  explicit BasicRendererException(const std::string &msg) :
      runtime_error(msg) {}
};

class BasicRenderer {
private:
  Shader color_shader;
  Texture single_black_pixel_texture;
  WindowEventProducer *event_producer = nullptr;

  bool debug_mode = true;

public:
  BasicRenderer();

  void render(Camera &camera, entt::registry &world);

  void set_texture_with_default(std::string name, int location,
                                const Texture *texture) const;
};
} // namespace ale

#endif // BASIC_RENDERER_H
