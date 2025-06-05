module;

#include <entt/entt.hpp>
#include <format>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include "src/graphics/sdf/sdf_model_packed.h"
#include "src/graphics/static_mesh.h"

export module basic_renderer;
import transform;
import file_system;
import light;
import camera;
import window_event;
import shader;

using afs = ale::FileSystem;
using namespace std;
using namespace glm;

export namespace ale::graphics::renderer {
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
  BasicRenderer() :
      color_shader(
          afs::root("resources/shaders/renderer/basic_renderer.vs").c_str(),
          afs::root("resources/shaders/renderer/basic_renderer.fs").c_str()),
      single_black_pixel_texture(
          afs::root("resources/textures/default/black1x1.png")) {}

  void render(Camera &camera, entt::registry &world) {
    glClearColor(135.0 / 255, 206.0 / 255, 235.0 / 255, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    color_shader.use();
    color_shader.setMat4("projection", camera.get_projection_matrix());
    color_shader.setMat4("view", camera.get_view_matrix());
    color_shader.setVec3("viewPos", camera.Position);

    auto light_view = world.view<Transform, Light>();
    int light_index = 0;
    for (const auto &[entity, transform, light]: light_view.each()) {
      color_shader.setVec3(format("lights[{}].position", light_index),
                           transform.translation);
      color_shader.setVec3(format("lights[{}].color", light_index),
                           light.color);
      color_shader.setFloat(format("lights[{}].radius", light_index),
                            light.radius);
      color_shader.setVec3(format("lights[{}].attenuation", light_index),
                           light.attenuation);
      light_index += 1;
    }
    color_shader.setInt("numLights", light_index);

    // Handle shadows, can only handle 1 sdf model packed for now.
    auto sdf_model_packed = std::shared_ptr<SdfModelPacked>(nullptr);
    auto entries = vector<pair<Transform, vector<unsigned int>>>();
    auto shadow_view = world.view<Transform, StaticMesh>();
    for (auto [entity, transform, static_mesh]: shadow_view.each()) {
      auto [packed, packed_index] = static_mesh.get_model_shadow();
      if (static_mesh.get_cast_shadow() && packed != nullptr) {
        if (sdf_model_packed != nullptr &&
            sdf_model_packed.get() != packed.get()) {
          throw BasicRendererException("multiple different sdf model packed on "
                                       "basic renderer not supported");
        }
        sdf_model_packed = packed;
        entries.emplace_back(transform, packed_index);
      }
    }
    if (sdf_model_packed != nullptr) {
      sdf_model_packed->bind_to_shader(color_shader, entries, 5);
    }
    // End handle shadows

    // Render static mesh
    const auto view = world.view<Transform, StaticMesh, BasicMaterial>();
    for (auto [entity, transform, static_mesh, material]: view.each()) {
      color_shader.setMat4("model", transform.get_model_matrix());

      color_shader.setVec4("diffuseColor", vec4(1.0, 1.0, 1.0, 0.0));
      set_texture_with_default("diffuseTexture", 0,
                               material.diffuse_texture.get());

      static_mesh.get_model()->draw(color_shader);
    }
  }

  void set_texture_with_default(string name, int location,
                                const Texture *texture) const {

    color_shader.setTexture2D(name, location,
                              texture == nullptr ? single_black_pixel_texture.id
                                                 : texture->id);
  }
};
} // namespace ale::graphics::renderer
