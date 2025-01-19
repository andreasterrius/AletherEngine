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
    : gizmo_frame(Framebuffer::Meta{.width = initial_window_size.x,
                                    .height = initial_window_size.y,
                                    .color_space = Framebuffer::LINEAR}),
      content_browser_ui(sm_loader, afs::root("resources/content_browser")),
      scene_viewport_ui(initial_window_size),
      gizmo_light(make_pair(vec3(5.0f), Light{})),
      test_texture(afs::root("resources/textures/wood.png")) {}

void EditorRootLayout::start(ivec2 pos, ivec2 size) {
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
  show_menubar = (ImGui::Begin("Main", NULL, windowFlags)); // show the "window"
  ImGui::PopStyleVar(); // restore the style so inner windows have fames

  dockspace_id = ImGui::DockSpace(ImGui::GetID("Dockspace"), ImVec2(0.0f, 0.0f),
                                  ImGuiDockNodeFlags_PassthruCentralNode);
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
void EditorRootLayout::start_capture_scene(Camera &camera) {
  {
    gizmo_frame.start_capture();
    gizmo.render(camera, gizmo_light.first);
    gizmo_frame.end_capture();
  }

  scene_viewport_ui.start_capture();
}

void EditorRootLayout::end_capture_scene() {
  texture_renderer.render(*gizmo_frame.get_color_attachment0(),
                          TextureRenderer::RenderMeta{.discard_alpha = true});
  scene_viewport_ui.end_capture();
}
void EditorRootLayout::debug() {
  glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  texture_renderer.render(*gizmo_frame.get_color_attachment0(),
                          TextureRenderer::RenderMeta{.discard_alpha = false});
}

EditorRootLayout::Event
EditorRootLayout::draw_and_handle_events(entt::registry &world) {

  Event event;
  if (show_menubar) {
    // Do a menu bar with an exit menu
    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New"))
          event.is_new_clicked = true;
        if (ImGui::MenuItem("Exit"))
          event.is_exit_clicked = true;
        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();
    }
  }

  event.new_object = content_browser_ui.draw_and_handle_clicks();

  // Show the scene
  scene_viewport_ui.draw();
  scene_tree_ui.draw_and_handle_clicks(world, gizmo.selected_entity);

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

  return event;
}

} // namespace ale::ui
