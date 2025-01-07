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

//clang-format off
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
//clang-format on

using namespace std;
using namespace ale;
using afs = ale::FileSystem;

int main() {
  glfwInit();

  auto screen_size = ivec2(1200, 800);
  auto window = Window(screen_size.x, screen_size.y, "Hello Imgui");

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

  ImGui_ImplGlfw_InitForOpenGL(
      window.get(), true); // Second param install_callback=true will install
                           // GLFW callbacks and chain to existing ones.
  ImGui_ImplOpenGL3_Init();

  while (!window.should_close()) {
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::ShowDemoWindow(); // Show demo window! :)

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    window.swap_buffer_and_poll_inputs();
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwTerminate();
  return 0;
}
