import material;
import deferred_renderer;
import util;
import logger;
import stash;
import command;
import history_stack;
import content_browser;
import editor_root;
import item_inspector;

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "src/data/file_system.h"
#include "src/data/scene_node.h"
#include "src/data/serde/world.h"
#include "src/graphics/camera.h"
#include "src/graphics/gizmo/gizmo.h"
#include "src/graphics/light.h"
#include "src/graphics/line_renderer.h"
#include "src/graphics/static_mesh.h"
#include "src/graphics/thumbnail_generator.h"
#include "src/graphics/window.h"

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

  // Declare UI related
  auto editor_root_layout_ui = editor::EditorRoot(sm_loader, window.get_size());
  auto world = editor_root_layout_ui.new_world(sm_loader);

  camera.add_listener(&window);
  editor_root_layout_ui.add_listener(&window);

  while (!window.get_should_close()) {
    // Input stuff
    camera.set_handle_input(editor_root_layout_ui.get_scene_has_focus());

    editor_root_layout_ui.set_tick_data(editor::EditorRoot::TickData{
        .camera = &camera,
        .world = &world,
        .cursor_pos_topleft = window.get_cursor_pos_from_top_left()});
    editor_root_layout_ui.tick();

    // Render Scene
    deferred_renderer.render_first_pass(camera, world);

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
      editor_root_layout_ui.draw_and_handle_cmds(window, sm_loader, world);
      window.end_ui_frame();
    }

    window.swap_buffer_and_poll_inputs();
  }

  glfwTerminate();
  return 0;
}
