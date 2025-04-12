// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <entt/entt.hpp>
#include <nfd.hpp>

#include "src/data/file_system.h"
#include "src/graphics/camera.h"
#include "src/graphics/model.h"
#include "src/graphics/renderer/basic_renderer.h"
#include "src/graphics/sdf/sdf_generator_gpu.h"
#include "src/graphics/sdf/sdf_model_packed.h"
#include "src/graphics/static_mesh.h"
#include "src/graphics/window.h"

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
  NFD_Init();

  auto screen_size = glm::ivec2(1200, 800);
  auto window = Window(screen_size.x, screen_size.y, "Hello Imgui");

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |=
          ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  io.ConfigFlags |=
          ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;  // Enable Docking

  bool show_demo_window = true;
  bool show_another_window = false;
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  while (!window.get_should_close()) {
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

    {
      static float f = 0.0f;
      static int counter = 0;

      ImGui::Begin("Hello, world!");  // Create a window called "Hello, world!"
      // and append into it.

      ImGui::Text("This is some useful text.");  // Display some text (you can
      // use a format strings too)
      ImGui::Checkbox("Demo Window",
                      &show_demo_window);  // Edit bools storing our window
                                           // open/close state
      ImGui::Checkbox("Another Window", &show_another_window);

      ImGui::SliderFloat(
              "float", &f, 0.0f,
              1.0f);  // Edit 1 float using a slider from 0.0f to 1.0f
      ImGui::ColorEdit3(
              "clear color",
              (float *) &clear_color);  // Edit 3 floats representing a color

      if (ImGui::Button(
                  "Button"))  // Buttons return true when clicked (most
                              // widgets return true when edited/activated)
        counter++;
      ImGui::SameLine();
      ImGui::Text("counter = %d", counter);

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                  1000.0f / io.Framerate, io.Framerate);

      if (ImGui::Button("Open File")) {
        NFD::UniquePath outPath;
        nfdresult_t result = NFD::OpenDialog(outPath, nullptr);

        if (result == NFD_OKAY) {
          std::cout << "Selected file: " << outPath.get() << std::endl;
        } else if (result == NFD_CANCEL) {
          std::cout << "User canceled the dialog." << std::endl;
        }
      }

      ImGui::End();
    }

    // 3. Show another simple window.
    if (show_another_window) {
      ImGui::Begin("Another Window",
                   &show_another_window);  // Pass a pointer to our bool
                                           // variable (the
      // window will have a closing button that will
      // clear the bool when clicked)
      ImGui::Text("Hello from another window!");
      if (ImGui::Button("Close Me")) show_another_window = false;
      ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      GLFWwindow *backup_current_context = glfwGetCurrentContext();

      // Update and Render additional Platform Windows
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();

      glfwMakeContextCurrent(backup_current_context);
    }

    window.swap_buffer_and_poll_inputs();
  }

  NFD_Quit();
  glfwTerminate();
  return 0;
}
