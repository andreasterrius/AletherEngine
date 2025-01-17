//
// Created by Alether on 1/10/2025.
//

#ifndef ROOT_LAYOUT_H
#define ROOT_LAYOUT_H
#include "content_browser.h"
#include "scene_tree.h"
#include "scene_viewport.h"
#include "src/gizmo/gizmo.h"

#include <imgui.h>

namespace ale::ui {
class EditorRootLayout {
public:
  struct Event {
    bool is_exit_clicked = false;
    bool is_new_clicked = false;
  };

private:
  ContentBrowser content_browser_ui;
  SceneViewport scene_viewport_ui;
  SceneTree scene_tree_ui;
  Gizmo gizmo;
  pair<vec3, Light> gizmo_light;

private:
  ImGuiID dockspace_id;

public:
  EditorRootLayout(StaticMeshLoader &sm_loader, ivec2 initial_window_size);

  Event start(ivec2 pos, ivec2 size);

  void end();

  void handle_press(Camera &camera, entt::registry &world,
                    ivec2 cursor_top_left);
  void handle_release();
  void tick(Camera &camera, entt::registry &world, ivec2 cursor_top_left);
  void start_frame();
  void end_frame(Camera &camera);
  void draw_and_handle_events(entt::registry &world);
};
} // namespace ale::ui

#endif // ROOT_LAYOUT_H
