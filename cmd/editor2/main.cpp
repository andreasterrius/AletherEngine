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

Ray getMouseRay(vec2 logical_pos, mat4 projMat, mat4 viewMat) {
  vec4 rayStartNdc =
      vec4((logical_pos.x * 2) - 1, (logical_pos.y * 2) - 1, -1.0f, 1.0f);
  vec4 rayEndNdc =
      vec4((logical_pos.x * 2) - 1, (logical_pos.y * 2) - 1, 0.0f, 1.0f);

  // not sure why this has to be inverted.
  rayStartNdc.y *= -1;
  rayEndNdc.y *= -1;

  mat4 invViewProj = inverse(projMat * viewMat);
  vec4 rayStartWorld = invViewProj * rayStartNdc;
  vec4 rayEndWorld = invViewProj * rayEndNdc;

  rayStartWorld /= rayStartWorld.w;
  rayEndWorld /= rayEndWorld.w;

  Ray r(rayStartWorld, normalize(rayEndWorld - rayStartWorld));

  return r;
}

int main() {
  glfwInit();

  auto window = Window(1280, 800, "Editor 2");
  auto camera = Camera(ARCBALL, window.get_size().first,
                       window.get_size().second, glm::vec3(3.0f, 5.0f, -7.0f));
  auto lights = vector{Light{vec3(5.0f, 5.0f, 5.0f)}};

  // Declare a basic scene
  auto basic_renderer = BasicRenderer();
  auto line_renderer = LineRenderer();

  auto sm_loader = StaticMeshLoader();
  auto sm_monkey =
      sm_loader.load_static_mesh(afs::root("resources/models/monkey.obj"));
  auto sm_floor =
      sm_loader.load_static_mesh(afs::root("resources/models/floor_cube.obj"));
  auto gizmo = Gizmo();

  // Create world
  auto world = entt::registry{};
  {
    const auto entity = world.create();
    world.emplace<Transform>(entity, Transform{});
    world.emplace<StaticMesh>(entity, sm_monkey);
  }
  {
    const auto entity = world.create();
    world.emplace<Transform>(entity, Transform{
                                         .translation = vec3(0.0, -5.0, 0.0),
                                     });
    world.emplace<StaticMesh>(entity, sm_floor);
  }

  // Declare UI related
  auto content_browser_ui =
      ui::ContentBrowser(sm_loader, afs::root("resources/content_browser"));
  auto scene_viewport_ui = ui::SceneViewport(
      ivec2(window.get_size().first, window.get_size().second));
  auto editor_root_layout_ui = ui::EditorRootLayout{};

  // Mouse
  auto mouse_ray = Ray(vec3(), vec3(0.0, 0.0, 1.0));

  // Attach event listeners here
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

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
      auto cursor_pos = window.get_cursor_pos_from_top_left();
      auto logical_cursor_pos = scene_viewport_ui.convert_to_logical_pos(
          ivec2(cursor_pos.first, cursor_pos.second));
      auto [wx, wy] = window.get_size();
      mouse_ray = getMouseRay(logical_cursor_pos, camera.GetProjectionMatrix(),
                              camera.GetViewMatrix());
    }
  });

  while (!window.should_close()) {
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gizmo.tryHold();

    // Render Scene
    {
      scene_viewport_ui.start_frame();
      basic_renderer.render(camera, lights, world);

      line_renderer.queue_line(mouse_ray, WHITE);
      line_renderer.render(camera.GetProjectionMatrix(),
                           camera.GetViewMatrix());
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
