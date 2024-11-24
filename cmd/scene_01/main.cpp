// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include "src/camera.h"
#include "src/components/renderable.h"
#include "src/data/model.h"
#include "src/renderer/basic_renderer.h"
#include "src/sdf_generator_gpu.h"
#include "src/sdf_model_packed.h"
#include "src/window.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "src/file_system.h"

using namespace std;
using namespace ale;
using afs = ale::FileSystem;

int main() {
  glfwInit();

  auto window = Window(800, 600, "Scene 01");
  auto camera = Camera(ARCBALL, glm::vec3(0.0f, 0.0f, 10.0f));
  auto basic_renderer = BasicRenderer();

  auto lights = vector<Light>{};

  auto monkey = Model(afs::root("resources/models/monkey.obj"));
  auto monkey_sdf = SdfModel(monkey, Texture3D::load("monkey64"), 64);

  auto unit_cube = Model(afs::root("resources/models/unit_cube.obj"));
  auto unit_cube_sdf = SdfModel(unit_cube, 16);

  SdfModelPacked packedSdfModel(vector<SdfModel*>{&monkey_sdf, &unit_cube_sdf});

  while (!window.should_close()) {
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // basic_renderer.render(camera, lights, renderables);

    window.swap_buffer_and_poll_inputs();
  }

  glfwTerminate();
  return 0;
}
