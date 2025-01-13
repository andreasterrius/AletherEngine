// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include "src/data/static_mesh.h"
#include "src/file_system.h"
#include "src/gizmo/gizmo.h"
#include "src/renderer/line_renderer.h"
#include "src/renderer/thumbnail_generator.h"
#include "src/renderer/ui/content_browser.h"
#include "src/renderer/ui/editor_root_layout.h"
#include "src/renderer/ui/scene_viewport.h"
#include "src/window.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

// clang-format off
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
// clang-format on

using namespace std;
using namespace ale;
using namespace glm;
using afs = ale::FileSystem;

int main() {
  glfwInit();

  auto window = Window(1280, 800, "Editor 2");
  auto camera = Camera(ARCBALL, window.get_size().x, window.get_size().y,
                       glm::vec3(3.0f, 5.0f, -7.0f));
  auto lights = vector{Light{vec3(5.0f, 5.0f, 5.0f)}};

  // Declare a basic scene
  auto basic_renderer = BasicRenderer();
  auto line_renderer = LineRenderer();

  auto sm_loader = StaticMeshLoader();
  auto sm_monkey =
      sm_loader.load_static_mesh(afs::root("resources/models/monkey.obj"));
  auto gizmo = Gizmo();

  // Create world
  auto world = entt::registry{};
  {
    const auto entity = world.create();
    world.emplace<Transform>(entity, Transform{});
    world.emplace<StaticMesh>(entity, sm_monkey);
  }
  // {
  //   const auto entity = world.create();
  //   world.emplace<Transform>(entity, Transform{
  //                                        .translation = vec3(0.0, -5.0, 0.0),
  //                                    });
  //   world.emplace<StaticMesh>(entity, sm_floor);
  // }

  // Declare UI related
  auto content_browser_ui =
      ui::ContentBrowser(sm_loader, afs::root("resources/content_browser"));
  auto scene_viewport_ui =
      ui::SceneViewport(ivec2(window.get_size().x, window.get_size().y));
  auto editor_root_layout_ui = ui::EditorRootLayout{};

  // Attach event listeners here
  window.attach_mouse_button_callback([&](int button, int action, int mods) {
    // Release if we are holding something
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
      gizmo.handle_release();
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
      Ray mouse_ray = scene_viewport_ui.create_mouse_ray(
          window.get_cursor_pos_from_top_left(), camera.GetProjectionMatrix(),
          camera.GetViewMatrix());
      gizmo.handle_press(mouse_ray, world);
    }
  });
  window.attach_cursor_pos_callback(
      [&](double xpos, double ypos, double xoffset, double yoffset) {
        camera.ProcessMouseMovement(xoffset, yoffset);
      });
  window.attach_scroll_callback([&](double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
  });
  window.attach_key_callback([&](int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_LEFT_ALT && action == GLFW_PRESS)
      camera.ProcessKeyboardArcball(true);
    else if (key == GLFW_KEY_LEFT_ALT && action == GLFW_RELEASE)
      camera.ProcessKeyboardArcball(false);
  });

  while (!window.should_close()) {
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Ray mouse_ray = scene_viewport_ui.create_mouse_ray(
        window.get_cursor_pos_from_top_left(), camera.GetProjectionMatrix(),
        camera.GetViewMatrix());
    gizmo.tick(mouse_ray, world);

    // Render Scene
    {
      scene_viewport_ui.start_frame();
      basic_renderer.render(camera, lights, world);

      // line_renderer.queue_line(debug_ray, WHITE);
      line_renderer.render(camera.GetProjectionMatrix(),
                           camera.GetViewMatrix());

      glDisable(GL_DEPTH_TEST);
      gizmo.render(camera, lights[0].position);
      glEnable(GL_DEPTH_TEST);
      // for (int i = 0; i < 9; ++i) {
      //   line_renderer.queue_box(gizmo.transform,
      //                           gizmo.models[i].meshes[0].boundingBox,
      //                           WHITE);
      // }

      scene_viewport_ui.end_frame();
    }

    // Render UI
    {
      window.start_ui_frame();

      auto events =
          editor_root_layout_ui.start(window.get_position(), window.get_size());
      if (events.is_exit_clicked) {
        window.set_should_close(true);
      }

      // Show and handle content browser
      auto clicked = content_browser_ui.draw_and_handle_clicks();
      if (clicked.has_value() && clicked->static_mesh.has_value()) {
        const auto entity = world.create();
        world.emplace<Transform>(entity, Transform{});
        world.emplace<StaticMesh>(entity, *clicked->static_mesh);
      }

      // Show the scene
      scene_viewport_ui.draw();

      editor_root_layout_ui.end();
      window.end_ui_frame();
    }

    window.swap_buffer_and_poll_inputs();
  }

  glfwTerminate();
  return 0;
}
