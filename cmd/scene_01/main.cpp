#include <glad/glad.h>
#include <glfw/glfw3.h>
#include "src/camera.h"
#include "src/data/model.h"
#include "src/components/renderable.h"
#include "src/renderer/basic_renderer.h"
#include "src/sdf_generator_gpu.h"
#include "src/window.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "src/file_system.h"

using namespace std;
using namespace ale;
using afs = ale::FileSystem;

int main()
{
    glfwInit();

    auto window = Window(800, 600, "Scene 01");
    auto camera = Camera(ARCBALL, glm::vec3(0.0f, 0.0f, 10.0f));
    auto basic_renderer = BasicRenderer();

    auto lights = vector<Light>{};

    auto monkey = Model(afs::root("resources/models/monkey.obj"));
    auto unit_cube = Model(afs::root("resources/models/unit_cube.obj"));

    auto renderables = vector<Renderable>{};
    renderables.emplace_back(Transform{}, SDFShadowMeta{.resolution = 8}, make_shared<Model>(monkey));
    renderables.emplace_back(Transform{}, SDFShadowMeta{.resolution = 8}, make_shared<Model>(unit_cube));

    basic_renderer.prepare_shadowable_objects(renderables);

    while (!window.should_close())
    {
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        basic_renderer.render(camera, lights, renderables);

        window.swap_buffer_and_poll_inputs();
    }

    glfwTerminate();
    return 0;
}
