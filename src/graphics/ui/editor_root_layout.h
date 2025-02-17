//
// Created by Alether on 1/10/2025.
//

#ifndef ROOT_LAYOUT_H
#define ROOT_LAYOUT_H

#include "content_browser.h"
#include "item_inspector.h"
#include "scene_tree.h"
#include "scene_viewport.h"
#include "src/graphics/gizmo/gizmo.h"
#include <imgui.h>

namespace ale::ui {
class EditorRootLayout {
public:
  struct Event {
    bool is_exit_clicked = false;
    bool is_new_clicked = false;
    optional<ContentBrowser::Entry> new_object;
  };

private:
  ItemInspector item_inspector;
  ContentBrowser content_browser_ui;
  SceneViewport scene_viewport_ui;
  SceneTree scene_tree_ui;

  TextureRenderer texture_renderer;
  Framebuffer gizmo_frame;
  Gizmo gizmo;
  pair<glm::vec3, Light> gizmo_light;

  Texture test_texture;

private:
  ImGuiID dockspace_id;
  bool show_menubar;
  bool scene_has_focus;

public:
  EditorRootLayout(StaticMeshLoader &sm_loader, glm::ivec2 initial_window_size);

  void start(glm::ivec2 pos, glm::ivec2 size);
  Event draw_and_handle_events(entt::registry &world);
  void end();

  // input
  void handle_press(Camera &camera, entt::registry &world,
                    glm::ivec2 cursor_top_left);
  void handle_release();
  void handle_key(int key, int scancode, int action, int mods);

  void tick(Camera &camera, entt::registry &world, glm::ivec2 cursor_top_left);
  void start_capture_scene(Camera &camera);
  void end_capture_scene();

  bool get_scene_has_focus();

  void debug();
};
} // namespace ale::ui

#endif // ROOT_LAYOUT_H
