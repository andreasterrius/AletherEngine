#include "basic_renderer.h"

#include "../data/static_mesh.h"
#include "../file_system.h"

using afs = ale::FileSystem;
using namespace ale;

BasicRenderer::BasicRenderer()
    : basicShader(afs::root("src/renderer/basic_renderer.vs").c_str(),
                  afs::root("src/renderer/basic_renderer.fs").c_str()) {
  glEnable(GL_CULL_FACE);
}

void ale::BasicRenderer::render(Camera &camera, vector<Light> &lights,
                                entt::registry &world) {
  basicShader.use();
  basicShader.setMat4("projection", camera.GetProjectionMatrix());
  basicShader.setMat4("view", camera.GetViewMatrix());
  basicShader.setVec3("viewPos", camera.Position);

  if (!lights.empty())
    basicShader.setVec3("lightPos", lights[0].position);
  else
    basicShader.setVec3("lightPos", vec3());

  // Handle shadows
  auto sdf_model_packed = std::shared_ptr<SdfModelPacked>(nullptr);
  auto entries = vector<pair<Transform, unsigned int>>();
  auto shadow_view = world.view<Transform, StaticMesh>();
  for (auto [entity, transform, static_mesh] : shadow_view.each()) {
    if (!static_mesh.get_sdf_shadow().has_value()) {
      continue;
    }
    if (sdf_model_packed == nullptr) {
      sdf_model_packed = static_mesh.get_sdf_shadow()->first;
    }
    entries.emplace_back(transform, static_mesh.get_sdf_shadow()->second);
  }

  if (sdf_model_packed != nullptr) {
    sdf_model_packed->bind_to_shader(basicShader, entries);
  }
  // End handle shadows

  auto view = world.view<Transform, StaticMesh>();
  for (auto [entity, transform, static_mesh] : view.each()) {
    basicShader.setMat4("model", transform.getModelMatrix());
    basicShader.setVec4("diffuseColor", vec4(1.0, 1.0, 0.0, 0.0));
    static_mesh.get_model()->draw(basicShader);
  }
}
