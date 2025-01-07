// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <entt/entt.hpp>

#include "src/camera.h"
#include "src/data/model.h"
#include "src/data/static_mesh.h"
#include "src/file_system.h"
#include "src/renderer/basic_renderer.h"
#include "src/sdf_generator_gpu.h"
#include "src/sdf_model_packed.h"
#include "src/window.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace std;
using namespace ale;
using afs = ale::FileSystem;

int main() {
  glfwInit();

  auto screen_size = ivec2(800, 600);
  auto window = Window(screen_size.x, screen_size.y, "Scene 01");
  auto camera = Camera(ARCBALL, screen_size.x, screen_size.y,
                       glm::vec3(3.0f, 5.0f, -7.0f));

  window.attach_cursor_pos_callback(
      [&](double xpos, double ypos, double xoffset, double yoffset) {
        camera.ProcessMouseMovement(xoffset, yoffset);
      });
  window.attach_scroll_callback([&](double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
  });

  auto lights = vector<Light>{Light{vec3(5.0f, 5.0f, 5.0f)}};
  auto basic_renderer = BasicRenderer();

  // auto monkey_model =
  //     make_shared<Model>(afs::root("resources/models/monkey.obj"));
  // auto floor_cube_model =
  //     make_shared<Model>(afs::root("resources/models/floor_cube.obj"));
  //
  // auto monkey_sdf =
  //     SdfModel(monkey_model->meshes[0], Texture3D::load("monkey64"), 64);
  // auto unit_cube_sdf = SdfModel(floor_cube_model->meshes[0], 64);
  //
  // auto sdf_model_packed =
  //     make_shared<SdfModelPacked>(vector{&monkey_sdf, &unit_cube_sdf});

  auto sm_loader = StaticMeshLoader();
  auto sm_monkey =
      sm_loader.load_static_mesh(afs::root("resources/models/monkey.obj"));
  auto sm_floor =
      sm_loader.load_static_mesh(afs::root("resources/models/floor_cube.obj"));

  auto world = entt::registry{};
  {
    const auto entity = world.create();
    world.emplace<Transform>(entity, Transform{});
    world.emplace<StaticMesh>(entity, sm_monkey);
  }
  {
    const auto entity = world.create();
    world.emplace<Transform>(entity, Transform{
                                         .translation = vec3(0.0, -5.0, 0.0),
                                     });
    world.emplace<StaticMesh>(entity, sm_floor);
  }

  while (!window.should_close()) {
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    basic_renderer.render(camera, lights, world);

    if (glfwGetKey(window.get(), GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
      camera.ProcessKeyboardArcball(true);
    else if (glfwGetKey(window.get(), GLFW_KEY_LEFT_ALT) == GLFW_RELEASE)
      camera.ProcessKeyboardArcball(false);

    window.swap_buffer_and_poll_inputs();
  }

  glfwTerminate();
  return 0;
}
