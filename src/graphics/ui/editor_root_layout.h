//
// Created by Alether on 1/10/2025.
//

#ifndef ROOT_LAYOUT_H
#define ROOT_LAYOUT_H

import item_inspector;

#include "content_browser.h"
#include "scene_tree.h"
#include "scene_viewport.h"
#include "src/graphics/gizmo/gizmo.h"
#include "src/graphics/light.h"
#include "src/graphics/window.h"
#include <imgui.h>

namespace ale::ui {
class EditorRootLayout : public WindowEventListener {
public:
  struct Event {
    bool is_exit_clicked = false;
    bool is_new_clicked = false;
    optional<ContentBrowser::Entry> new_object;
    ItemInspector::Event item_inspector_event;
  };

  // Data needed in a frame
  // All pointers are non owning
  struct TickData {
    Camera *camera;
    entt::registry *world;
    glm::ivec2 cursor_pos_topleft;
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
  TickData tick_data;

private:
  ImGuiID dockspace_id;
  bool show_menubar;
  bool scene_has_focus;

  // non owning
  WindowEventProducer *event_producer = nullptr;

public:
  EditorRootLayout(StaticMeshLoader &sm_loader, glm::ivec2 initial_window_size);
  ~EditorRootLayout();

  void start(glm::ivec2 pos, glm::ivec2 size);
  Event draw_and_handle_events(entt::registry &world);
  void end();

  // input
  void handle_press(Camera &camera, entt::registry &world,
                    glm::ivec2 cursor_top_left);
  void handle_release();

  void tick();
  void capture_scene(std::function<void()> exec, Camera &camera);
  void start_capture_scene(Camera &camera);
  void end_capture_scene();

  bool get_scene_has_focus();

  void set_tick_data(TickData frame);

  void debug();
  void add_listener(Window *window);

public:
  void mouse_button_callback(int button, int action, int mods) override;
  void cursor_pos_callback(double xpos, double ypos, double xoffset,
                           double yoffset) override;
  void framebuffer_size_callback(int width, int height) override;
  void scroll_callback(double x_offset, double y_offset) override;
  void key_callback(int key, int scancode, int action, int mods) override;
};
} // namespace ale::ui

#endif // ROOT_LAYOUT_H
