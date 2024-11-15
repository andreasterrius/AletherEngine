#include <glad/glad.h>
#include <glfw/glfw3.h>
#include "src/camera.h"
#include "src/data/model.h"
#include "src/data/scene.h"
#include "src/renderer/basic_renderer.h"
#include "src/sdf_generator_gpu.h"
#include "src/window.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "src/file_system.h"

using namespace ale;
using afs = ale::FileSystem;

struct Character {
  string name;
};
struct Enemy {};

int main() {
  glfwInit();

  auto window = Window(800, 600, "Scene 01");

  BasicRenderer renderer;

  SceneNode root;
  root.data = make_shared<std::any>(Character{"hero"});

  while (!window.should_close()) {
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    window.swap_buffer_and_poll_inputs();
  }

  glfwTerminate();
  return 0;
}
