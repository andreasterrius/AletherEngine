//
// Created by Alether on 1/10/2025.
//

#include "editor_root_layout.h"

#include "src/data/scene_node.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <spdlog/spdlog.h>

namespace ale::ui {

EditorRootLayout::EditorRootLayout(StaticMeshLoader &sm_loader,
                                   ivec2 initial_window_size)
    : content_browser_ui(sm_loader, afs::root("resources/content_browser")),
      scene_viewport_ui(initial_window_size),
      gizmo_light(vec3(5.0f, 5.0f, 5.0f)) {}

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

  dockspace_id = ImGui::DockSpace(ImGui::GetID("Dockspace"), ImVec2(0.0f, 0.0f),
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

void EditorRootLayout::handle_press(Camera &camera, entt::registry &world,
                                    ivec2 cursor_top_left) {
  Ray mouse_ray = scene_viewport_ui.create_mouse_ray(
      cursor_top_left, camera.GetProjectionMatrix(), camera.GetViewMatrix());
  gizmo.handle_press(mouse_ray, world);
}

void EditorRootLayout::handle_release() { gizmo.handle_release(); }

void EditorRootLayout::tick(Camera &camera, entt::registry &world,
                            ivec2 cursor_top_left) {
  Ray mouse_ray = scene_viewport_ui.create_mouse_ray(
      cursor_top_left, camera.GetProjectionMatrix(), camera.GetViewMatrix());
  gizmo.tick(mouse_ray, world);
}
void EditorRootLayout::start_frame() { scene_viewport_ui.start_frame(); }

void EditorRootLayout::end_frame(Camera &camera) {
  glDisable(GL_DEPTH_TEST);
  gizmo.render(camera, gizmo_light.position);
  glEnable(GL_DEPTH_TEST);

  scene_viewport_ui.end_frame();
}
void EditorRootLayout::draw_and_handle_events(entt::registry &world) {
  auto clicked = content_browser_ui.draw_and_handle_clicks();
  if (clicked.has_value() && clicked->static_mesh.has_value()) {
    const auto entity = world.create();
    world.emplace<SceneNode>(entity, SceneNode("unnamed"));
    world.emplace<Transform>(entity, Transform{});
    world.emplace<StaticMesh>(entity, *clicked->static_mesh);
  }

  // Show the scene
  scene_viewport_ui.draw();
  scene_tree_ui.draw_and_handle_clicks(world);

  // Assign specific windows to each dock
  ImGui::DockBuilderAddNode(dockspace_id,
                            ImGuiDockNodeFlags_DockSpace); // Create dockspace
  ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

  // Layout the scene
  ImGuiID dock_main = dockspace_id; // Main dock area
  ImGuiID dock_left = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Left,
                                                  0.25f, nullptr, &dock_main);
  ImGuiID dock_right = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Right,
                                                   0.25f, nullptr, &dock_main);

  ImGui::DockBuilderDockWindow(scene_tree_ui.panel_name.c_str(), dock_left);
  ImGui::DockBuilderDockWindow(content_browser_ui.panel_name.c_str(),
                               dock_right);
  ImGui::DockBuilderDockWindow(scene_viewport_ui.panel_name.c_str(), dock_main);

  // Finalize the layout
  ImGui::DockBuilderFinish(dockspace_id);
}

} // namespace ale::ui
