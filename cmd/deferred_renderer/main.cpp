import material;
import deferred_renderer;

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <glm/glm.hpp>

#include <entt/entt.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

import camera;
import compute_shader;
import model;
import mesh;
import static_mesh;
import transform;
import shader;
import file_system;
import window;
import deferred_renderer;
import stash;
import texture;
import light;

using namespace std;
using namespace glm;
using namespace ale;
using namespace ale::graphics;
using namespace ale::graphics::renderer;
using namespace ale::data;

using namespace std;
using namespace ale;
using namespace glm;
using afs = ale::FileSystem;

int main() {
  glfwInit();

  auto screen_size = ivec2(1200, 800);
  auto window = Window(screen_size.x, screen_size.y, "Scene 01");
  auto camera = Camera(ARCBALL, screen_size.x, screen_size.y,
                       glm::vec3(3.0f, 5.0f, -7.0f));
  camera.add_listener(&window);

  auto texture_stash = make_shared<Stash<Texture>>();
  auto deferred_renderer = DeferredRenderer(window.get_size());
  auto sm_loader = StaticMeshLoader(texture_stash);
  auto sm_monkey =
      sm_loader.load_static_mesh(afs::root("resources/models/monkey.obj"));
  auto sm_floor =
      sm_loader.load_static_mesh(afs::root("resources/models/floor_cube.obj"));
  auto sm_unit_cube =
      sm_loader.load_static_mesh(afs::root("resources/models/unit_cube.obj"));

  deferred_renderer.add_listener(&window);

  auto world = entt::registry{};
  {
    const auto entity = world.create();
    world.emplace<Transform>(entity, Transform{});
    world.emplace<StaticMesh>(entity, sm_monkey);
    world.emplace<BasicMaterial>(entity, BasicMaterial{});
  }
  {
    const auto entity = world.create();
    world.emplace<Transform>(entity, Transform{
                                         .translation = vec3(0.0, -5.0, 0.0),
                                     });
    world.emplace<StaticMesh>(entity, sm_floor);
    world.emplace<BasicMaterial>(entity, BasicMaterial{});
  }
  {
    const auto entity = world.create();
    world.emplace<Transform>(entity, Transform{vec3(5.0f, 5.0f, 5.0f)});
    // world.emplace<StaticMesh>(entity, sm_unit_cube);
    world.emplace<BasicMaterial>(
        entity, BasicMaterial{.diffuse_color = vec3(1.0f, 0.0f, 0.0f)});
    world.emplace<Light>(entity, Light{vec3(5.0f, 5.0f, 5.0f)});
  }

  while (!window.get_should_close()) {
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    deferred_renderer.render_both_pass(camera, world);

    window.swap_buffer_and_poll_inputs();
  }

  glfwTerminate();
  return 0;
}
