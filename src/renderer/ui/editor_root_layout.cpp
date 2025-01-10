//
// Created by Alether on 1/10/2025.
//

#include "editor_root_layout.h"
#include <imgui.h>

namespace ale::ui {

EditorRootLayout::Event EditorRootLayout::start(ivec2 pos, ivec2 size) {
  auto x = pos.x;
  auto y = pos.y;
  auto size_x = size.x;
  auto size_y = size.y;
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
          return Event{.is_exit_clicked = true};
        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();
    }
  }

  return Event{};
}

void EditorRootLayout::end() { ImGui::End(); }

} // namespace ale::ui
