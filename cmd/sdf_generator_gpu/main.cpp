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
    auto window = Window(1024, 768, "SDF Generator");

    Model sample(afs::root("resources/models/sample.obj"));

    SDFGeneratorGPU sdfgen;
    sdfgen.add("sample", sample.meshes[0], 32, 32, 32);
    sdfgen.generate();

    bool should_close = false;
    while(!should_close){
        // polls and wait

        // if(sdfgen.complete()){
        //  should_close = true;
        // }

        window.swap_buffer_and_poll_inputs();
    }

    sdfgen.dump("resources/somefolder");

    glfwTerminate();
    return 0;
}
