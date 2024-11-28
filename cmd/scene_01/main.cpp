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
                       glm::vec3(10.0f, 10.0f, 10.0f));
  auto basic_renderer = BasicRenderer();
  auto lights = vector<Light>{Light{vec3(5.0f, 5.0f, 5.0f)}};

  auto monkey = make_shared<Model>(afs::root("resources/models/monkey.obj"));
  auto monkey_sdf =
      make_shared<SdfModel>(*monkey, Texture3D::load("monkey64"), 64);

  auto unit_cube =
      make_shared<Model>(afs::root("resources/models/unit_cube.obj"));
  auto unit_cube_sdf = make_shared<SdfModel>(*unit_cube, 16);

  auto sdf_model_packed = make_shared<SdfModelPacked>(
      vector<SdfModel*>{monkey_sdf.get(), unit_cube_sdf.get()});

  auto monkey_mesh = StaticMesh(monkey, monkey_sdf, sdf_model_packed);
  auto unit_cube_mesh = StaticMesh(unit_cube, unit_cube_sdf, sdf_model_packed);

  auto world = entt::registry{};
  // {
  //   const auto entity = world.create();
  //   world.emplace<Transform>(entity, Transform{});
  //   world.emplace<StaticMesh>(entity, monkey_mesh);
  // }
  {
    const auto entity = world.create();
    world.emplace<Transform>(entity, Transform{});
    world.emplace<StaticMesh>(entity, unit_cube_mesh);
  }

  while (!window.should_close()) {
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    basic_renderer.render(camera, lights, world);

    window.swap_buffer_and_poll_inputs();
  }

  glfwTerminate();
  return 0;
}
