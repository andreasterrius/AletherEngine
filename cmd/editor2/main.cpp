import material;
import deferred_renderer;
import history_stack;
import util;
import logger;
import stash;

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "src/data/file_system.h"
#include "src/data/scene_node.h"
#include "src/data/serde/world.h"
#include "src/editor/content_browser.h"
#include "src/editor/editor_root_layout.h"
#include "src/graphics/camera.h"
#include "src/graphics/gizmo/gizmo.h"
#include "src/graphics/light.h"
#include "src/graphics/line_renderer.h"
#include "src/graphics/static_mesh.h"
#include "src/graphics/thumbnail_generator.h"
#include "src/graphics/window.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// clang-format off
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
// clang-format on

using namespace std;
using namespace ale;
using namespace glm;
using afs = ale::FileSystem;

entt::registry new_world(StaticMeshLoader &sm_loader) {
  // Create world
  auto world = entt::registry{};

  // Lights
  {
    const auto entity = world.create();
    world.emplace<SceneNode>(entity, SceneNode("light"));
    world.emplace<Transform>(entity,
                             Transform{.translation = vec3(10.0, 10.0, 10.0)});
    world.emplace<Light>(entity, Light{});
    world.emplace<BasicMaterial>(entity, BasicMaterial{});

    auto sphere = *sm_loader.get_static_mesh(SM_UNIT_SPHERE);
    sphere.set_cast_shadow(false);
    world.emplace<StaticMesh>(entity, sphere);
  }
  {
    const auto entity = world.create();
    world.emplace<SceneNode>(entity, SceneNode("light"));
    world.emplace<Transform>(entity,
                             Transform{.translation = vec3(10.0, 10.0, -10.0)});
    world.emplace<Light>(entity, Light{.color = vec3(3.0f, 3.0f, 3.0f)});
    world.emplace<BasicMaterial>(entity, BasicMaterial{});

    auto sphere = *sm_loader.get_static_mesh(SM_UNIT_SPHERE);
    sphere.set_cast_shadow(false);
    world.emplace<StaticMesh>(entity, sphere);
  }
  return world;
}

void handle_editor_cmds(
    Window &window, StaticMeshLoader &sm_loader, entt::registry &world,
    editor::HistoryStack &history_stack,
    std::vector<editor::EditorRootLayout::Cmd> &editor_cmds) {
  while (!editor_cmds.empty()) {
    auto cmd = editor_cmds.back();
    editor_cmds.pop_back();

    match(
        cmd,
        [&](editor::EditorRootLayout::ExitCmd &arg) {
          window.set_should_close(true);
        },
        [&](editor::EditorRootLayout::NewWorldCmd &arg) {
          world.clear<>();
          world = new_world(sm_loader);
        },
        [&](editor::EditorRootLayout::NewObjectCmd &arg) {
          const auto entity = world.create();
          world.emplace<SceneNode>(
              entity,
              arg.new_object.static_mesh.get_model()->path.filename().string());
          world.emplace<Transform>(entity, Transform{});
          world.emplace<StaticMesh>(entity, arg.new_object.static_mesh);
          world.emplace<BasicMaterial>(entity, arg.new_object.basic_material);
        },
        [&](editor::ItemInspector::Cmd &arg) {
          match(arg, [&](editor::ItemInspector::LoadTextureCmd &arg) {
            auto texture = make_shared<Texture>(arg.path);
            auto basic_material =
                world.try_get<BasicMaterial>(arg.entity_to_load);
            if (editor::DIFFUSE == arg.type) {
              basic_material->add_diffuse(texture);
            } else if (editor::SPECULAR == arg.type) {
              basic_material->add_specular(texture);
            }
          });
        },
        [&](editor::EditorRootLayout::TransformChangeHistoryCmd &arg) {},
        [&](editor::EditorRootLayout::UndoCmd &arg) {
          std::cout << "Undo cmd called" << std::endl;
        },
        [&](editor::EditorRootLayout::RedoCmd &arg) {
          std::cout << "Redo cmd called" << std::endl;
        },
        [&](editor::EditorRootLayout::SaveWorldCmd &arg) {
          std::cout << "Save world called" << std::endl;
          serde::save_world("temp/scenes/editor2.json", world);
        },
        [&](editor::EditorRootLayout::LoadWorldCmd &arg) {
          std::cout << "Load world called" << std::endl;
          try {
            world = serde::load_world("temp/scenes/editor2.json", sm_loader);
          } catch (const std::exception &e) {
            ale::logger::get()->info("{}", e.what());
          }
        });
  }
}

int main() {
  glfwInit();

  ale::logger::init();

  auto window = Window(1280, 800, "Editor 2");
  auto camera = Camera(ARCBALL, window.get_size().x, window.get_size().y,
                       glm::vec3(3.0f, 5.0f, 7.0f));

  // Stashes
  auto texture_stash = make_shared<Stash<Texture>>();

  // Declare a basic scene
  auto deferred_renderer = DeferredRenderer(window.get_size());
  auto texture_renderer = TextureRenderer();
  auto line_renderer = LineRenderer();
  auto sm_loader = StaticMeshLoader(texture_stash);
  auto history_stack = editor::HistoryStack();

  auto world = new_world(sm_loader);

  // Declare UI related
  auto editor_root_layout_ui =
      editor::EditorRootLayout(sm_loader, window.get_size());

  camera.add_listener(&window);
  editor_root_layout_ui.add_listener(&window);

  while (!window.get_should_close()) {
    glClearColor(135.0 / 255, 206.0 / 255, 235.0 / 255, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Input stuff
    camera.set_handle_input(editor_root_layout_ui.get_scene_has_focus());
    editor_root_layout_ui.set_tick_data(editor::EditorRootLayout::TickData{
        .camera = &camera,
        .world = &world,
        .cursor_pos_topleft = window.get_cursor_pos_from_top_left()});
    editor_root_layout_ui.tick();

    deferred_renderer.render_first_pass(camera, world);

    // Render Scene
    editor_root_layout_ui.capture_scene(
        [&]() {
          deferred_renderer.render_second_pass(camera, world);

          line_renderer.render(camera.get_projection_matrix(),
                               camera.get_view_matrix());
        },
        camera);

    // Render UI
    {
      window.start_ui_frame();
      editor_root_layout_ui.start(window.get_position(), window.get_size());

      auto cmds = editor_root_layout_ui.draw_and_handle_cmds(world);
      handle_editor_cmds(window, sm_loader, world, history_stack, cmds);

      editor_root_layout_ui.end();
      window.end_ui_frame();
    }

    window.swap_buffer_and_poll_inputs();
  }

  glfwTerminate();
  return 0;
}
