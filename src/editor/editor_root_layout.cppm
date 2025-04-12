//
// Created by Alether on 1/10/2025.
//
module;

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <spdlog/spdlog.h>
#include "scene_tree.h"
#include "scene_viewport.h"
#include "src/data/file_system.h"
#include "src/graphics/camera.h"
#include "src/graphics/gizmo/gizmo.h"
#include "src/graphics/light.h"
#include "src/graphics/static_mesh.h"
#include "src/graphics/window.h"

export module editor_root_layout;

import item_inspector;
import command_stack;
import command;
import content_browser;

export namespace ale::editor {
using namespace glm;
using namespace std;

class EditorRootLayout : public WindowEventListener {
public:
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

  vector<Cmd> callback_cmds;

public:
  using afs = ale::FileSystem;

  EditorRootLayout(StaticMeshLoader &sm_loader, ivec2 initial_window_size) :
      gizmo_frame(Framebuffer::Meta{.width = initial_window_size.x,
                                    .height = initial_window_size.y,
                                    .color_space = Framebuffer::LINEAR}),
      content_browser_ui(sm_loader,
                         afs::root("resources/models/content_browser")),
      scene_viewport_ui(initial_window_size),
      gizmo_light(make_pair(vec3(5.0f), Light{})),
      test_texture(afs::root("resources/textures/wood.png")),
      scene_has_focus(false) {}

  ~EditorRootLayout() {
    if (this->event_producer) {
      this->event_producer->remove_listener(this);
    }
  }

  void start(ivec2 pos, ivec2 size) {
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
    show_menubar =
        (ImGui::Begin("Main", NULL, windowFlags)); // show the "window"
    ImGui::PopStyleVar(); // restore the style so inner windows have fames

    dockspace_id =
        ImGui::DockSpace(ImGui::GetID("Dockspace"), ImVec2(0.0f, 0.0f),
                         ImGuiDockNodeFlags_PassthruCentralNode);
  }

  void end() { ImGui::End(); }

  void handle_press(Camera &camera, entt::registry &world,
                    ivec2 cursor_top_left) {
    if (scene_viewport_ui.is_cursor_inside(cursor_top_left)) {
      Ray mouse_ray = scene_viewport_ui.create_mouse_ray(
          cursor_top_left, camera.get_projection_matrix(),
          camera.get_view_matrix());
      gizmo.handle_press(mouse_ray, camera, cursor_top_left, world);
    }
  }

  void handle_release() { gizmo.handle_release(); }

  void tick() {
    auto &t = tick_data;
    Ray mouse_ray = scene_viewport_ui.create_mouse_ray(
        t.cursor_pos_topleft, t.camera->get_projection_matrix(),
        t.camera->get_view_matrix());
    gizmo.tick(mouse_ray, *t.camera, t.cursor_pos_topleft, *t.world);

    scene_has_focus = scene_viewport_ui.is_cursor_inside(t.cursor_pos_topleft);
  }
  void capture_scene(std::function<void()> exec, Camera &camera) {
    start_capture_scene(camera);
    exec();
    end_capture_scene();
  }

  void start_capture_scene(Camera &camera) {
    {
      gizmo_frame.start_capture();
      gizmo.render(camera, gizmo_light.first);
      gizmo_frame.end_capture();
    }

    scene_viewport_ui.start_capture();
  }

  void end_capture_scene() {
    texture_renderer.render(
        *gizmo_frame.get_color_attachment0(),
        TextureRenderer::RenderMeta{.discard_alpha = true,
                                    .enable_blending = false});
    scene_viewport_ui.end_capture();
  }

  bool get_scene_has_focus() { return scene_has_focus; }

  void set_tick_data(TickData tick_data) { this->tick_data = tick_data; }

  void debug() {
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    texture_renderer.render(
        *gizmo_frame.get_color_attachment0(),
        TextureRenderer::RenderMeta{.discard_alpha = false});
  }

  void add_listener(Window *window) {
    this->event_producer = window;
    window->add_listener(this);
  }

  vector<Cmd> draw_and_handle_cmds(entt::registry &world) {

    vector<Cmd> cmds;

    // if (auto release_info = gizmo.get_release_info()) {
    //   auto [a, b, c] = release_info.value();
    //   event.transform_change_cmd = TransformChangeCommand{a, b, c};
    //   events.push_back(TransformChangeCmd{a, b, c});
    // }

    if (show_menubar) {
      // Do a menu bar with an exit menu
      if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
          if (ImGui::MenuItem("New"))
            cmds.emplace_back(NewWorldCmd{});
          if (ImGui::MenuItem("Exit"))
            cmds.emplace_back(ExitCmd{});
          ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
      }
    }

    if (auto entry = content_browser_ui.draw_and_handle_clicks()) {
      cmds.emplace_back(NewObjectCmd{*entry});
    }

    // Show the scene
    scene_viewport_ui.draw();

    for (auto &cmd:
         item_inspector.draw_and_handle(world, gizmo.selected_entity)) {
      cmds.emplace_back(cmd);
    }

    scene_tree_ui.draw_and_handle_clicks(world, gizmo.selected_entity);

    // Assign specific windows to each dock
    ImGui::DockBuilderAddNode(dockspace_id,
                              ImGuiDockNodeFlags_DockSpace); // Create dockspace
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

    // Layout the scene
    ImGuiID dock_main = dockspace_id; // Main dock area
    ImGuiID dock_left = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Left,
                                                    0.25f, nullptr, &dock_main);
    ImGuiID dock_right = ImGui::DockBuilderSplitNode(
        dock_main, ImGuiDir_Right, 0.25f, nullptr, &dock_main);
    ImGuiID dock_bottom = ImGui::DockBuilderSplitNode(
        dock_main, ImGuiDir_Down, 0.25f, nullptr, &dock_main);

    ImGui::DockBuilderDockWindow(scene_tree_ui.panel_name.c_str(), dock_left);
    ImGui::DockBuilderDockWindow(item_inspector.panel_name.c_str(), dock_right);
    ImGui::DockBuilderDockWindow(scene_viewport_ui.panel_name.c_str(),
                                 dock_main);
    ImGui::DockBuilderDockWindow(content_browser_ui.panel_name.c_str(),
                                 dock_bottom);

    // Finalize the layout
    ImGui::DockBuilderFinish(dockspace_id);

    cmds.insert(cmds.end(), callback_cmds.begin(), callback_cmds.end());
    callback_cmds.clear();

    return cmds;
  }

  void mouse_button_callback(int button, int action, int mods) {
    auto &t = tick_data;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
      handle_release();
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
      handle_press(*t.camera, *t.world, t.cursor_pos_topleft);
    }
  }
  void cursor_pos_callback(double xpos, double ypos, double xoffset,
                           double yoffset) {}
  void framebuffer_size_callback(int width, int height) {}
  void scroll_callback(double x_offset, double y_offset) {}
  void key_callback(int key, int scancode, int action, int mods) {
    // Handle Gizmo
    if (!ImGui::GetIO().WantTextInput) {
      if (key == GLFW_KEY_W && action == GLFW_PRESS) {
        gizmo.change_mode(Translate);
      } else if (key == GLFW_KEY_E && action == GLFW_PRESS) {
        gizmo.change_mode(Scale);
      } else if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        gizmo.change_mode(Rotate);
      }
    }

    if (get_scene_has_focus()) {
      if (key == GLFW_KEY_S && action == GLFW_PRESS &&
          mods == GLFW_MOD_CONTROL) {
        callback_cmds.emplace_back(SaveWorldCmd{});
      }
      if (key == GLFW_KEY_N && action == GLFW_PRESS &&
          mods == GLFW_MOD_CONTROL) {
        callback_cmds.emplace_back(NewWorldCmd{});
      }
      if (key == GLFW_KEY_O && action == GLFW_PRESS &&
          mods == GLFW_MOD_CONTROL) {
        callback_cmds.emplace_back(LoadWorldCmd{});
      }
      if (key == GLFW_KEY_Z && action == GLFW_PRESS &&
          mods == GLFW_MOD_CONTROL) {
        callback_cmds.emplace_back(UndoCmd{});
      } else if (key == GLFW_KEY_Z && action == GLFW_PRESS &&
                 (mods & GLFW_MOD_CONTROL) && (mods & GLFW_MOD_SHIFT)) {
        callback_cmds.emplace_back(RedoCmd{});
      }
    }
  }
};
} // namespace ale::editor
