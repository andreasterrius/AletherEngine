// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include "src/data/static_mesh.h"
#include "src/file_system.h"
#include "src/renderer/thumbnail_generator.h"
#include "src/renderer/ui/content_browser.h"
#include "src/window.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

// clang-format off
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
// clang-format on

using namespace std;
using namespace ale;
using namespace glm;
using afs = ale::FileSystem;

int main() {
  glfwInit();

  auto window = Window(1280, 800, "Editor 2");
  auto camera = Camera(ARCBALL, window.get_size().first,
                       window.get_size().second, glm::vec3(3.0f, 5.0f, -7.0f));

  // Declare a basic scene
  auto basic_renderer = BasicRenderer();
  auto lights = vector{Light{vec3(5.0f, 5.0f, 5.0f)}};
  auto sm_loader = StaticMeshLoader();
  auto sm_monkey =
      sm_loader.load_static_mesh(afs::root("resources/models/monkey.obj"));
  auto sm_floor =
      sm_loader.load_static_mesh(afs::root("resources/models/floor_cube.obj"));

  // Create world
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

  // Declare UI related
  auto content_browser_ui =
      ui::ContentBrowser(sm_loader, afs::root("resources/content_browser"));
  auto framebuffer = Framebuffer(
      Framebuffer::Meta{window.get_size().first, window.get_size().second});

  // Attach event listeners here
  window.attach_cursor_pos_callback(
      [&](double xpos, double ypos, double xoffset, double yoffset) {
        camera.ProcessMouseMovement(xoffset, yoffset);
      });
  window.attach_scroll_callback([&](double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
  });
  window.attach_key_callback([&](int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_LEFT_ALT && action == GLFW_PRESS)
      camera.ProcessKeyboardArcball(true);
    else if (key == GLFW_KEY_LEFT_ALT && action == GLFW_RELEASE)
      camera.ProcessKeyboardArcball(false);
  });

  while (!window.should_close()) {
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render Scene
    {
      framebuffer.start_frame();

      basic_renderer.render(camera, lights, world);

      framebuffer.end_frame();
    }

    // Render UI
    {
      window.start_ui_frame();

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
      bool show_menubar =
          (ImGui::Begin("Main", NULL, windowFlags)); // show the "window"
      ImGui::PopStyleVar(); // restore the style so inner windows have fames

      ImGui::DockSpace(ImGui::GetID("Dockspace"), ImVec2(0.0f, 0.0f),
                       ImGuiDockNodeFlags_PassthruCentralNode);
      if (show_menubar) {
        // Do a menu bar with an exit menu
        if (ImGui::BeginMenuBar()) {
          if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit"))
              window.set_should_close(true);

            ImGui::EndMenu();
          }
          ImGui::EndMenuBar();
        }
      }

      // Show and handle content browser
      auto clicked = content_browser_ui.draw_and_handle_clicks();
      if (clicked.has_value() && clicked->static_mesh.has_value()) {
        const auto entity = world.create();
        world.emplace<Transform>(entity, Transform{});
        world.emplace<StaticMesh>(entity, *clicked->static_mesh);
      }

      // Show the scene
      {
        ImGui::Begin("Scene Viewport");
        ImVec2 window_size = ImGui::GetContentRegionAvail();
        ImGui::Image((GLuint)framebuffer.get_color_attachment0()->id,
                     window_size, ImVec2(0, 1), ImVec2(1, 0));
        ImGui::End();
      }

      ImGui::End();

      window.end_ui_frame();
    }

    window.swap_buffer_and_poll_inputs();
  }

  glfwTerminate();
  return 0;
}
