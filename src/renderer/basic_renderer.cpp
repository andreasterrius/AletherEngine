#include "basic_renderer.h"

#include "../data/static_mesh.h"
#include "../file_system.h"

using afs = ale::FileSystem;
using namespace ale;

BasicRenderer::BasicRenderer()
    : basic_shader(afs::root("src/renderer/basic_renderer.vs").c_str(),
                   afs::root("src/renderer/basic_renderer.fs").c_str()) {
  glEnable(GL_CULL_FACE);
}

void ale::BasicRenderer::render(Camera &camera, vector<Light> &lights,
                                entt::registry &world) {
  basic_shader.use();
  basic_shader.setMat4("projection", camera.GetProjectionMatrix());
  basic_shader.setMat4("view", camera.GetViewMatrix());
  basic_shader.setVec3("viewPos", camera.Position);

  if (!lights.empty())
    basic_shader.setVec3("lightPos", lights[0].position);
  else
    basic_shader.setVec3("lightPos", vec3());

  auto view = world.view<Transform, StaticMesh>();
  for (auto [entity, transform, static_mesh] : view.each()) {
    basic_shader.setMat4("model", transform.getModelMatrix());
    basic_shader.setVec4("diffuseColor", vec4(1.0, 1.0, 0.0, 0.0));
    static_mesh.get_model()->draw(basic_shader);
  }
}
