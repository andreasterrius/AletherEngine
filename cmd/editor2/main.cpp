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

  auto window = Window(1280, 800, "Editor 2");
  while (!window.should_close()) {
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    {
      window.start_frame();

      // ImGui::ShowDemoWindow();

      auto [x, y] = window.get_position();
      auto [size_x, size_y] = window.get_size();
      ImGui::SetNextWindowPos(ImVec2(x, y)); // always at the window origin
      ImGui::SetNextWindowSize(ImVec2(size_x, size_y));
      ImGuiWindowFlags windowFlags =
          ImGuiWindowFlags_NoBringToFrontOnFocus | // we just want to use this
                                                   // window as a host for the
                                                   // menubar and docking
          ImGuiWindowFlags_NoNavFocus | // so turn off everything that would
                                        // make it act like a window
          ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
          ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
          ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar |
          ImGuiWindowFlags_NoBackground; // we want our game content to show
                                         // through this window, so turn off the
                                         // background.

      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                          ImVec2(0, 0)); // we don't want any padding for
                                         // windows docked to this window frame
      bool show =
          (ImGui::Begin("Main", NULL, windowFlags)); // show the "window"
      ImGui::PopStyleVar(); // restore the style so inner windows have fames

      ImGui::DockSpace(ImGui::GetID("Dockspace"), ImVec2(0.0f, 0.0f),
                       ImGuiDockNodeFlags_PassthruCentralNode);
      if (show) {
        // Do a menu bar with an exit menu
        if (ImGui::BeginMenuBar()) {
          if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit"))
              std::cout << "Exit";

            ImGui::EndMenu();
          }
          ImGui::EndMenuBar();
        }
      }

      ImGui::End();

      window.end_frame();
    }

    window.swap_buffer_and_poll_inputs();
  }

  glfwTerminate();
  return 0;
}
