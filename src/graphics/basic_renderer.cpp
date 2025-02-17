#include "basic_renderer.h"

#include "../data/static_mesh.h"
#include "../file_system.h"
#include <format>
#include <spdlog/fmt/bundled/format.h>

namespace ale {

using namespace glm;
using afs = ale::FileSystem;

BasicRenderer::BasicRenderer()
    : color_shader(afs::root("src/graphics/basic_renderer.vs").c_str(),
                   afs::root("src/graphics/basic_renderer.fs").c_str()) {
  glEnable(GL_CULL_FACE);
}

void BasicRenderer::render(Camera &camera, entt::registry &world) {

  glClearColor(135.0 / 255, 206.0 / 255, 235.0 / 255, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  color_shader.use();
  color_shader.setMat4("projection", camera.GetProjectionMatrix());
  color_shader.setMat4("view", camera.GetViewMatrix());
  color_shader.setVec3("viewPos", camera.Position);

  auto light_view = world.view<Transform, Light>();
  int light_index = 0;
  for (auto [entity, transform, light] : light_view.each()) {
    color_shader.setVec3(format("lights[{}].position", light_index),
                         transform.translation);
    color_shader.setVec3(format("lights[{}].color", light_index), light.color);
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
  for (auto [entity, transform, static_mesh] : shadow_view.each()) {
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
    sdf_model_packed->bind_to_shader(color_shader, entries);
  }
  // End handle shadows

  // Render static mesh
  auto view = world.view<Transform, StaticMesh>();
  for (auto [entity, transform, static_mesh] : view.each()) {
    color_shader.setMat4("model", transform.get_model_matrix());
    color_shader.setVec4("diffuseColor", vec4(1.0, 1.0, 1.0, 0.0));
    static_mesh.get_model()->draw(color_shader);
  }
}
} // namespace ale