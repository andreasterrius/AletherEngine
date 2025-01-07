// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include "src/file_system.h"
#include "src/window.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#define STB_IMAGE_IMPLEMENTATION
#include <iostream>

#include <ostream>
#include <stb_image.h>

using namespace std;
using namespace ale;
using namespace glm;
using afs = ale::FileSystem;

int main() {
  glfwInit();

  auto screen_size = ivec2(1200, 800);
  auto window = Window(screen_size.x, screen_size.y, "Editor 2");

  while (!window.should_close()) {
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    {
      window.start_frame();

      ImGui::ShowDemoWindow();
      //TODO: experimen with layouting the editor, perhaps we can also do hot-reloading?

      window.end_frame();
    }

    window.swap_buffer_and_poll_inputs();
  }

  glfwTerminate();
  return 0;
}
