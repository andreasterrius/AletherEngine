#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <src/window.h>
#include "src/sdf_generator_gpu.h"
#include "src/data/model.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "src/file_system.h"

using namespace ale;
using afs = ale::FileSystem;

int main() {
    glfwInit();
    auto window = Window(32, 32, "SDF Generator");

    Model sample(afs::root("resources/models/sample.obj"));

    SDFGeneratorGPU sdfgen;
    sdfgen.add("sample", sample.meshes[0], 32, 32, 32);
    sdfgen.generate();

    TextureRenderer texture_renderer;

    bool should_close = false;
    while (!window.should_close()){
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        texture_renderer.render(sdfgen.debug_result.at("sample"));

        window.swap_buffer_and_poll_inputs();
    }

    sdfgen.dump("resources/somefolder");

    glfwTerminate();
    return 0;
}
