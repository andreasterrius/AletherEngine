// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
#include "src/graphics/camera.h"
#include "src/graphics/model.h"
#include "src/graphics/sdf/sdf_generator_gpu.h"
#include "src/graphics/sdf/sdf_model.h"
#include <src/graphics/window.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "src/data/file_system.h"

using namespace ale;
using afs = ale::FileSystem;

int main() {
  glfwInit();
  auto window = Window(1024, 100, "SDF Generator");
  window.set_debug(true);
  Model sample(afs::root("resources/models/monkey.obj"));
  Model unit_cube(afs::root("resources/models/unit_cube.obj"));

  SdfGeneratorGPU sdfgen;
  // sdfgen.add_mesh("monkey64", sample.meshes[0], 64, 64, 64);
  // sdfgen.add_mesh("monkey32", sample.meshes[0], 32, 32, 32);
  sdfgen.add_mesh("monkey16", sample.meshes[0], 16, 16, 16);
  // sdfgen.add_mesh("unit_cube32", unit_cube.meshes[0], 32, 32, 32);
  // sdfgen.add_mesh("unit_cube64", unit_cube.meshes[0], 64, 64, 64);
  sdfgen.generate_all();

  auto s = SdfModel(sample.meshes[0], 16);

  TextureRenderer texture_renderer;

  bool should_close = false;
  while (!window.should_close()) {
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    texture_renderer.render(sdfgen.debug_result.at("monkey16"));
    window.swap_buffer_and_poll_inputs();
  }

  // sdfgen.at("monkey64").save("monkey64");
  // sdfgen.at("monkey32").save("monkey32");
  sdfgen.at("monkey16").save("monkey16");
  // sdfgen.at("unit_cube32").save("unit_cube32");
  // sdfgen.at("unit_cube64").save("unit_cube64");

  sdfgen.dump_textfile("monkey16");
  s.texture3D->save_textfile("monkey_16cpu");

  glfwTerminate();
  return 0;
}
