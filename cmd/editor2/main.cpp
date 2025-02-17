// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include "spdlog/spdlog.h"
#include "src/data/scene_node.h"
#include "src/data/serde/world.h"
#include "src/data/static_mesh.h"
#include "src/file_system.h"
#include "src/gizmo/gizmo.h"
#include "src/graphics/line_renderer.h"
#include "src/graphics/thumbnail_generator.h"
#include "src/graphics/ui/content_browser.h"
#include "src/graphics/ui/editor_root_layout.h"
#include "src/window.h"
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

    auto sphere = *sm_loader.get_static_mesh(SM_UNIT_SPHERE);
    sphere.set_cast_shadow(false);
    world.emplace<StaticMesh>(entity, sphere);
  }
  return world;
}

int main() {
  glfwInit();

  spdlog::set_level(spdlog::level::trace);
  SPDLOG_INFO("Starting Editor2");

  auto window = Window(1280, 800, "Editor 2");
  auto camera = Camera(ARCBALL, window.get_size().x, window.get_size().y,
                       glm::vec3(3.0f, 5.0f, 7.0f));

  // Declare a basic scene
  auto basic_renderer = BasicRenderer();
  auto line_renderer = LineRenderer();
  auto sm_loader = StaticMeshLoader();

  auto world = new_world(sm_loader);

  // Declare UI related
  auto editor_root_layout_ui =
      ui::EditorRootLayout(sm_loader, window.get_size());

  // Attach event listeners here
  window.attach_mouse_button_callback([&](int button, int action, int mods) {
    // Release if we are holding something
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
      editor_root_layout_ui.handle_release();
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
      editor_root_layout_ui.handle_press(camera, world,
                                         window.get_cursor_pos_from_top_left());
    }
  });
  window.attach_cursor_pos_callback(
      [&](double xpos, double ypos, double xoffset, double yoffset) {
        if (editor_root_layout_ui.get_scene_has_focus()) {
          camera.ProcessMouseMovement(xoffset, yoffset);
        }
      });
  window.attach_scroll_callback([&](double xoffset, double yoffset) {
    if (editor_root_layout_ui.get_scene_has_focus()) {
      camera.ProcessMouseScroll(yoffset);
    }
  });
  window.attach_key_callback([&](int key, int scancode, int action, int mods) {
    if (editor_root_layout_ui.get_scene_has_focus()) {
      if (key == GLFW_KEY_LEFT_ALT && action == GLFW_PRESS)
        camera.ProcessKeyboardArcball(true);
      else if (key == GLFW_KEY_LEFT_ALT && action == GLFW_RELEASE)
        camera.ProcessKeyboardArcball(false);

      if (key == GLFW_KEY_S && action == GLFW_PRESS &&
          mods == GLFW_MOD_CONTROL) {
        serde::save_world("resources/scenes/editor2.json", world);
      }
      if (key == GLFW_KEY_N && action == GLFW_PRESS &&
          mods == GLFW_MOD_CONTROL) {
        world = new_world(sm_loader);
      }
      if (key == GLFW_KEY_O && action == GLFW_PRESS &&
          mods == GLFW_MOD_CONTROL) {
        try {
          world = serde::load_world("resources/scenes/editor2.json", sm_loader);
        } catch (const std::exception &e) {
          SPDLOG_ERROR("{}", e.what());
        }
      }
    }

    editor_root_layout_ui.handle_key(key, scancode, action, mods);
  });

  while (!window.should_close()) {
    glClearColor(135.0 / 255, 206.0 / 255, 235.0 / 255, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    editor_root_layout_ui.tick(camera, world,
                               window.get_cursor_pos_from_top_left());

    // Render Scene
    {
      editor_root_layout_ui.start_capture_scene(camera);
      basic_renderer.render(camera, world);

      // line_renderer.queue_line(debug_ray, WHITE);
      line_renderer.render(camera.GetProjectionMatrix(),
                           camera.GetViewMatrix());
      editor_root_layout_ui.end_capture_scene();
    }

    // Render UI
    {
      window.start_ui_frame();
      editor_root_layout_ui.start(window.get_position(), window.get_size());

      auto events = editor_root_layout_ui.draw_and_handle_events(world);
      // we can handle the events from UI here
      if (events.is_exit_clicked) {
        window.set_should_close(true);
      }
      if (events.is_new_clicked) {
        world.clear<>();
        world = new_world(sm_loader);
      }
      if (events.new_object.has_value()) {
        const auto entity = world.create();
        world.emplace<SceneNode>(entity,
                                 events.new_object->static_mesh.get_model()
                                     ->path.filename()
                                     .string());
        world.emplace<Transform>(entity, Transform{});
        world.emplace<StaticMesh>(entity, events.new_object->static_mesh);
      }

      editor_root_layout_ui.end();
      window.end_ui_frame();
    }

    window.swap_buffer_and_poll_inputs();
  }

  glfwTerminate();
  return 0;
}
