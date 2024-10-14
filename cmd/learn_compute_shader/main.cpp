#include <iostream>
#include <src/window.h>
#include <src/data/compute_shader.h>
#include <src/file_system.h>
#include <src/data/model.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "src/texture.h"

using namespace std;
using namespace glm;
using namespace ale;

using afs = ale::FileSystem;

int main() {
    glfwInit();
    auto window = Window(1024, 768, "LearnComputeShader");

    int maxWGCountX, maxWGCountY, maxWGCountZ;
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &maxWGCountX);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &maxWGCountY);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &maxWGCountZ);
    cout << "Compute Shader (GL_MAX_COMPUTE_WORK_GROUP_COUNT): (" << maxWGCountX << "," << maxWGCountY << "," <<
        maxWGCountZ << ")\n";

    int maxGroupSizeX, maxGroupSizeY, maxGroupSizeZ;
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &maxGroupSizeX);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &maxGroupSizeY);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &maxGroupSizeZ);
    cout << "Compute Shader (GL_MAX_COMPUTE_WORK_GROUP_SIZE): (" << maxGroupSizeX << "," << maxGroupSizeY << "," <<
        maxGroupSizeZ << ")\n";

    // compute shader
    ComputeShader testCompute(afs::root("src/shaders/sdf_compute_shader.cs"), 1024, 768);
    testCompute.execute();

    TextureRenderer textureRenderer;

    float deltaTime, lastFrame = glfwGetTime();
    while (!glfwWindowShouldClose(window.get())){
        // per-frame time logic
        // --------------------
        float currentFrame = (float)(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        testCompute.execute();
        textureRenderer.renderRaw(testCompute.textureId);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window.get());
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
