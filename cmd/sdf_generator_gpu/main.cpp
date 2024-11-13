#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <src/window.h>
#include "src/sdf_generator_gpu.h"
#include "src/data/model.h"
#include "src/camera.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "src/file_system.h"

using namespace ale;
using afs = ale::FileSystem;

int main() {
    glfwInit();
    auto window = Window(1024, 100, "SDF Generator");
    window.set_debug(true);
    Model sample(afs::root("resources/models/sample2.obj"));

    SDFGeneratorGPU sdfgen;
    sdfgen.add_mesh("sample", sample.meshes[0], 8, 8, 8);
    sdfgen.generate_all();

    TextureRenderer texture_renderer;

    bool should_close = false;
    while (!window.should_close()){
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        texture_renderer.render(sdfgen.debug_result.at("sample"));
        window.swap_buffer_and_poll_inputs();
    }

    auto t1 = sdfgen.at("sample");
    t1.save("test_save");
    t1.save_textfile("sample1");

    auto t2 = Texture3D::load("test_save");
    t2.save_textfile("sample2");

    glfwTerminate();
    return 0;
}
