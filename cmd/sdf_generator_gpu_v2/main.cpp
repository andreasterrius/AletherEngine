// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <src/window.h>

#include "src/camera.h"
#include "src/data/model.h"
#include "src/file_system.h"
#include "src/renderer/basic_renderer.h"
#include "src/renderer/line_renderer.h"
#include "src/renderer/sdf_generator_gpu_v2.h"
#include "src/sdf_model.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace ale;
using afs = ale::FileSystem;

int main() {
  glfwInit();

  auto window = Window(1024, 768, "SDF Generator V2");
  auto camera = Camera(ARCBALL, 1024, 768, glm::vec3(3.0f, 5.0f, -7.0f));
  auto basic_renderer = BasicRenderer();
  window.set_debug(true);

  Model model(afs::root("resources/models/monkey.obj"));

  SdfGeneratorGPUV2 sdfgen;
  LineRenderer line_renderer;

  auto texture = std::move(sdfgen.generate_gpu(model, 8).at(0));
  auto sdf_mesh = SdfModel(model.meshes.at(0), std::move(texture), 8);

  window.attach_cursor_pos_callback(
      [&](double xpos, double ypos, double xoffset, double yoffset) {
        camera.ProcessMouseMovement(xoffset, yoffset);
      });
  window.attach_scroll_callback([&](double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
  });

  while (!window.should_close()) {
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    line_renderer.queue_box(Transform{}, model.meshes[0].boundingBox,
                            vec3(1.0, 0.0, 0.0));
    sdf_mesh.loopOverCubes([&](int x, int y, int z, BoundingBox bb) {
      line_renderer.queue_box(Transform{}, bb, WHITE);
    });
    line_renderer.render(camera.GetProjectionMatrix(), camera.GetViewMatrix());

    if (glfwGetKey(window.get(), GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
      camera.ProcessKeyboardArcball(true);
    else if (glfwGetKey(window.get(), GLFW_KEY_LEFT_ALT) == GLFW_RELEASE)
      camera.ProcessKeyboardArcball(false);

    // texture_renderer.render(sdfgen.debug_result.at("monkey16"));
    window.swap_buffer_and_poll_inputs();
  }

  glfwTerminate();
  return 0;
}
