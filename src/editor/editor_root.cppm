//
// Created by Alether on 1/10/2025.
//
module;

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <entt/entt.hpp>
#include <filesystem>
#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <iostream>
#include <nfd.hpp>
#include <optional>
#include <spdlog/spdlog.h>
#include <vector>

export module editor:editor_root;
import graphics;
import serde;
import data;
import input;
import :item_inspector;
import :content_browser;
import :scene_viewport;
import :scene_tree;
import :history.history;
import :history.history_stack;
import :command;
import :dialog;
import util;

using namespace glm;
using namespace std;
using namespace ale::input;
using namespace ale::graphics;
using namespace ale::data;

export namespace ale::editor {

class EditorRoot : public WindowEventListener {
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

  history::HistoryStack history_stack;
  vector<Cmd> callback_cmds;

public:
  entt::registry new_world(StaticMeshLoader &sm_loader) {
    // Create world
    auto world = entt::registry{};

    // Lights
    {
      // Ambient Light
      const auto entity = world.create();
      world.emplace<SceneNode>(entity, SceneNode("ambient_light"));
      world.emplace<AmbientLight>(entity, AmbientLight{0.2f, WHITE, BLUE_SKY});
    }
    {
      const auto entity = world.create();
      world.emplace<SceneNode>(entity, SceneNode("light"));
      world.emplace<Transform>(
          entity, Transform{.translation = vec3(10.0, 10.0, 10.0)});
      world.emplace<Light>(entity, Light{});
      world.emplace<BasicMaterial>(entity, BasicMaterial{});

      auto sphere = *sm_loader.get_static_mesh(SM_UNIT_SPHERE);
      sphere.set_cast_shadow(false);
      world.emplace<StaticMesh>(entity, sphere);
    }
    {
      const auto entity = world.create();
      world.emplace<SceneNode>(entity, SceneNode("light"));
      world.emplace<Transform>(
          entity, Transform{.translation = vec3(10.0, 10.0, -10.0)});
      world.emplace<Light>(entity, Light{.color = vec3(3.0f, 3.0f, 3.0f)});
      world.emplace<BasicMaterial>(entity, BasicMaterial{});

      auto sphere = *sm_loader.get_static_mesh(SM_UNIT_SPHERE);
      sphere.set_cast_shadow(false);
      world.emplace<StaticMesh>(entity, sphere);
    }
    return world;
  }

public:
  EditorRoot(StaticMeshLoader &sm_loader,
             shared_ptr<Stash<Texture>> texture_stash,
             ivec2 initial_window_size) :
      gizmo_frame(Framebuffer::Meta{.width = initial_window_size.x,
                                    .height = initial_window_size.y,
                                    .color_space = Framebuffer::LINEAR}),
      content_browser_ui(sm_loader, texture_stash,
                         afs::root("resources/models/content_browser")),
      scene_viewport_ui(initial_window_size),
      gizmo_light(make_pair(vec3(5.0f), Light{})),
      test_texture(afs::root("resources/textures/wood.png")),
      scene_has_focus(false) {}

  ~EditorRoot() {
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

  void handle_release() {
    if (auto moved = gizmo.handle_release()) {
      auto [ent, b, a] = *moved;
      callback_cmds.emplace_back(TransformChangeNotif{ent, b, a});
    }
  }

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

  vector<Cmd> draw_and_handle_cmds(Window &window, StaticMeshLoader &sm_loader,
                                   entt::registry &world, Camera &camera) {
    start(window.get_position(), window.get_size());

    vector<Cmd> cmds;

    if (show_menubar) {
      // Do a menu bar with an exit menu
      if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
          if (ImGui::MenuItem("New"))
            cmds.emplace_back(NewWorldCmd{});
          if (ImGui::MenuItem("Save")) {
            cmds.emplace_back(SaveWorldCmd{nullopt});
          }
          if (ImGui::MenuItem("Load")) {
            cmds.emplace_back(LoadWorldCmd{nullopt});
          }
          if (ImGui::MenuItem("Exit"))
            cmds.emplace_back(ExitCmd{});
          ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
      }
    }

    if (auto entry = content_browser_ui.draw_and_handle_clicks(sm_loader)) {
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

    if (ImGui::BeginPopupModal("Quit Confirmation", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::Text("Are you sure you want to quit?");
      ImGui::Separator();

      if (ImGui::Button("Yes", ImVec2(120, 0))) {
        window.set_should_close(true);
      }

      ImGui::SetItemDefaultFocus();
      ImGui::SameLine();
      if (ImGui::Button("No", ImVec2(120, 0))) {
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }

    cmds.insert(cmds.end(), callback_cmds.begin(), callback_cmds.end());
    callback_cmds.clear();

    handle_editor_cmds(cmds, window, sm_loader, world, camera);

    camera.set_handle_input(get_scene_has_focus());

    end();
    return cmds;
  }

  void handle_editor_cmds(vector<Cmd> &cmds, Window &window,
                          StaticMeshLoader &sm_loader, entt::registry &world,
                          Camera &camera) {
    while (!cmds.empty()) {
      auto cmd = cmds.back();
      cmds.pop_back();

      match(
          cmd, [&](ExitCmd &arg) { ImGui::OpenPopup("Quit Confirmation"); },
          [&](NewWorldCmd &arg) {
            world.clear<>();
            world = new_world(sm_loader);
          },
          [&](NewObjectCmd &arg) {
            if (arg.new_object.static_mesh_with_material != nullopt) {

              const auto entity = world.create();
              world.emplace<SceneNode>(
                  entity,
                  arg.new_object.static_mesh_with_material->first.get_model()
                      ->path.filename()
                      .string());
              world.emplace<Transform>(entity, Transform{});
              world.emplace<StaticMesh>(
                  entity, arg.new_object.static_mesh_with_material->first);
              world.emplace<BasicMaterial>(
                  entity, arg.new_object.static_mesh_with_material->second);
            }
          },
          [&](ItemInspector::Cmd &arg) {
            match(arg, [&](ItemInspector::LoadTextureCmd &arg) {
              auto texture = make_shared<Texture>(arg.path);
              auto basic_material =
                  world.try_get<BasicMaterial>(arg.entity_to_load);
              if (DIFFUSE == arg.type) {
                basic_material->add_diffuse(texture);
              } else if (SPECULAR == arg.type) {
                basic_material->add_specular(texture);
              }
            });
          },
          [&](TransformChangeNotif &arg) {
            history_stack.add(make_unique<history::TransformHistory>(
                arg.entity, arg.before, arg.after));
          },
          [&](UndoCmd &arg) {
            std::cout << "Undo cmd called" << std::endl;
            history_stack.undo(world);
          },
          [&](RedoCmd &arg) {
            std::cout << "Redo cmd called" << std::endl;
            history_stack.redo(world);
          },
          [&](SaveWorldCmd &arg) {
            std::cout << "Save world called" << std::endl;
            string path;
            if (arg.path != nullopt) {
              path = *arg.path;
            } else {
              // open file picker
              if (auto p = save_file_picker(); p.has_value()) {
                path = *p;
              } else {
                return;
              }
            }

            try {
              serde::save_world(world, path);
            } catch (const std::exception &e) {
              std::cout << "Save world error, " << e.what();
              // logger::get()->info("{}", e.what());
            }
          },
          [&](LoadWorldCmd &arg) {
            std::cout << "Load world called" << std::endl;
            string path;
            if (arg.path != nullopt) {
              path = *arg.path;
            } else {
              // open file picker
              if (auto p = open_file_picker(); p.has_value()) {
                path = *p;
              } else {
                return;
              }
            }

            try {
              world = serde::load_world(path);
            } catch (const std::exception &e) {
              std::cout << "Load world error, " << e.what();
              // logger::get()->info("{}", e.what());
            }
          },
          [&](CameraLookAtEntityCmd &arg) {
            auto transform = world.try_get<Transform>(arg.entity);
            if (transform != nullptr) {
              camera.set_look_at(transform->translation);
            }
          });
    }
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

      if (key == GLFW_KEY_F && action == GLFW_PRESS &&
          gizmo.selected_entity != nullopt) {
        callback_cmds.emplace_back(
            CameraLookAtEntityCmd{*gizmo.selected_entity});
      }
    }
  }
};
} // namespace ale::editor
