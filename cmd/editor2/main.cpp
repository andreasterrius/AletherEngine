// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <entt/entt.hpp>
#include <filesystem>
#include <glm/glm.hpp>
#include <optional>
#include "spdlog/spdlog.h"

// clang-format off
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
// clang-format on

import data;
import graphics;
import editor;
import serde;

using namespace std;
using namespace ale;
using namespace ale::data;
using namespace ale::editor;
using namespace ale::graphics;
using namespace ale::graphics::renderer;
using namespace glm;


int main() {
  glfwInit();
  ale::logger::init();
  auto window = Window(1280, 800, "Editor 2");
  auto camera = Camera(ARCBALL, window.get_size().x, window.get_size().y,
                       glm::vec3(3.0f, 5.0f, 7.0f));

  auto texture_stash = make_shared<Stash<Texture>>();
  auto font_stash = make_shared<Stash<Font>>();

  // Declare a basic scene
  auto deferred_renderer = DeferredRenderer(window.get_size());
  auto texture_renderer = TextureRenderer();
  auto line_renderer = LineRenderer();
  auto sm_loader = StaticMeshLoader(texture_stash);

  // Declare UI related
  auto imgui = ImguiIntegration(&window, window.get_content_scale());
  auto editor_root =
      editor::EditorRoot(sm_loader, texture_stash, window.get_size());
  auto world = editor_root.new_world(sm_loader);
  camera.add_listener(&window);
  editor_root.add_listener(&window);
  while (!window.get_should_close()) {
    editor_root.set_tick_data(editor::EditorRoot::TickData{
        .camera = &camera,
        .world = &world,
        .cursor_pos_topleft = window.get_cursor_pos_from_top_left()});
    editor_root.tick();
    // Render Scene
    deferred_renderer.render_first_pass(camera, world);

    editor_root.capture_scene(
        [&]() {
          deferred_renderer.render_second_pass(camera, world);

          line_renderer.render(camera.get_projection_matrix(),
                               camera.get_view_matrix());
        },
        camera);

    // Render UI
    {
      imgui.start_frame();
      editor_root.draw_and_handle_cmds(window, sm_loader, world, camera);
      imgui.end_frame();
    }

    window.swap_buffer_and_poll_inputs();
  }

  glfwTerminate();
  return 0;
}
