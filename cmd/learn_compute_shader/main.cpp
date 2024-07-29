#include <iostream>
#include <memory>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include <src/renderer/line_renderer.h>
#include <src/window.h>
#include <src/data/shader.h>
#include <src/data/compute_shader.h>
#include <src/file_system.h>
#include <src/util.h>
#include <src/camera.h>
#include <src/data/ray.h>
#include <src/data/boundingbox.h>
#include <src/data/model.h>
#include <src/data/object.h>
#include <src/gizmo/gizmo.h>
#include <src/data/transform.h>
#include <src/sdf_model.h>

#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>

#include "src/texture.h"

using namespace std;
using namespace glm;
using namespace ale;

using afs = ale::FileSystem;

// should hold non owning datas
struct WindowData {
    Camera *camera;
    bool firstMouse;
    float lastX;
    float lastY;

    int screenWidth;
    int screenHeight;
    bool isCursorDisabled;
};

Ray getMouseRay(float mouseX, float mouseY,
                float screenWidth, float screenHeight,
                mat4 projMat, mat4 viewMat) {
    vec4 rayStartNdc = vec4(
        ((mouseX / screenWidth) * 2) - 1,
        ((mouseY / screenHeight) * 2) - 1,
        -1.0f,
        1.0f
    );
    vec4 rayEndNdc = vec4(
        ((mouseX / screenWidth) * 2) - 1,
        ((mouseY / screenHeight) * 2) - 1,
        0.0f,
        1.0f
    );

    // not sure why this has to be inverted.
    rayStartNdc.y *= -1;
    rayEndNdc.y *= -1;

    mat4 invViewProj = inverse(projMat * viewMat);
    vec4 rayStartWorld = invViewProj * rayStartNdc;
    vec4 rayEndWorld = invViewProj * rayEndNdc;

    rayStartWorld /= rayStartWorld.w;
    rayEndWorld /= rayEndWorld.w;

    Ray r(rayStartWorld, normalize(rayEndWorld - rayStartWorld));
    //    cout << r.toString() << "\n";

    return r;
}

vector<Ray> shootRaymarchRay(float screenWidth, float screenHeight, Camera &cam) {
    vector<Ray> rays;
    mat4 invViewProj = inverse(cam.GetProjectionMatrix(screenWidth, screenHeight) * cam.GetViewMatrix());
    for (float j = 0; j < screenHeight; ++j) {
        for (float i = 0; i < screenWidth; ++i) {
            vec2 uv = vec2(i / screenWidth * 2.0 - 1.0, j / screenHeight * 2.0 - 1.0);

            vec4 rayStartWorld = vec4(cam.Position, 1.0);
            vec4 rayEndWorld = invViewProj * vec4(uv, 0.0, 1.0);
            rayEndWorld /= rayEndWorld.w;

            vec4 rayDir = normalize(rayEndWorld - rayStartWorld);

            rays.push_back(Ray(rayStartWorld, rayDir));
        }
    }

    return rays;
}

Ray shootRaymarchRaySingular(float i, float j, float screenWidth, float screenHeight, Camera &cam) {
    mat4 invViewProj = inverse(cam.GetProjectionMatrix(screenWidth, screenHeight) * cam.GetViewMatrix());
    vec2 uv = vec2(i / screenWidth * 2.0 - 1.0, j / screenHeight * 2.0 - 1.0);

    vec4 rayStartWorld = vec4(cam.Position, 1.0);
    vec4 rayEndWorld = invViewProj * vec4(uv, 0.0, 1.0);
    rayEndWorld /= rayEndWorld.w;

    vec4 rayDir = normalize(rayEndWorld - rayStartWorld);

    return Ray(rayStartWorld, rayDir);
}

vector<vec4> raymarch(vector<Ray> &rays, SdfModel &sdf);

void renderScene(Shader &shader, vector<Object> &objects);

void renderCube();

//void renderBoundingBoxWireframe(LineRenderer &lr, Transform transform, BoundingBox bbT);

void processInput(GLFWwindow *window, float deltaTime, Camera &camera, bool &shadows, bool &shadowsKeyPressed);


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    auto *wd = (WindowData *) glfwGetWindowUserPointer(window);
    wd->screenWidth = width;
    wd->screenHeight = height;

    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void scrollCallback(GLFWwindow *window, double xOffset, double yOffset) {
    auto *wd = (WindowData *) glfwGetWindowUserPointer(window);
    wd->camera->ProcessMouseScroll(yOffset);
}

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        //getting cursor position
        glfwGetCursorPos(window, &xpos, &ypos);
        cout << "Cursor Position at (" << xpos << "," << ypos << ")" << endl;
    }


    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        auto *wd = (WindowData *) glfwGetWindowUserPointer(window);
        if (wd->isCursorDisabled) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        wd->isCursorDisabled = !wd->isCursorDisabled;
    }
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouseCallback(GLFWwindow *window, double xposIn, double yposIn) {
    auto *wd = (WindowData *) glfwGetWindowUserPointer(window);
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (wd->firstMouse) {
        wd->lastX = xpos;
        wd->lastY = ypos;
        wd->firstMouse = false;
    }

    float xoffset = xpos - wd->lastX;
    float yoffset = wd->lastY - ypos; // reversed since y-coordinates go from bottom to top

    wd->lastX = xpos;
    wd->lastY = ypos;

    wd->camera->ProcessMouseMovement(xoffset, yoffset);
}

int main() {
    Camera camera(ARCBALL, glm::vec3(3.0f, 3.0f, 10.0f));

    WindowData wd{
        .camera = &camera,
        .firstMouse = true,
        .screenWidth = 1000,
        .screenHeight = 1000,
        .isCursorDisabled = false
    };

    glfwInit();
    auto window = createWindow(wd.screenWidth, wd.screenHeight);
    if (window == nullptr) {
        glfwTerminate();
        cerr << "window not found";
        return -1;
    }
    glfwSetMouseButtonCallback(window.get(), mouseButtonCallback);
    glfwSetCursorPosCallback(window.get(), mouseCallback);
    glfwSetWindowUserPointer(window.get(), &wd);
    glfwSetFramebufferSizeCallback(window.get(), framebuffer_size_callback);
    glfwSetScrollCallback(window.get(), scrollCallback);

    Shader colorShader(afs::root("src/shaders/point_shadows.vs").c_str(),
                       afs::root("src/shaders/point_shadows.fs").c_str());
    Shader linearDepthShader(afs::root("src/shaders/point_shadows_depth.vs").c_str(),
                             afs::root("src/shaders/point_shadows_depth.fs").c_str(),
                             afs::root("src/shaders/point_shadows_depth.gs").c_str());

    int maxWGCountX, maxWGCountY, maxWGCountZ;
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &maxWGCountX);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &maxWGCountY);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &maxWGCountZ);
    cout << "Compute Shader (GL_MAX_COMPUTE_WORK_GROUP_COUNT): (" << maxWGCountX << "," << maxWGCountY << "," << maxWGCountZ << ")\n";

    int maxGroupSizeX, maxGroupSizeY, maxGroupSizeZ;
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &maxGroupSizeX);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &maxGroupSizeY);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &maxGroupSizeZ);
    cout << "Compute Shader (GL_MAX_COMPUTE_WORK_GROUP_SIZE): (" << maxGroupSizeX << "," << maxGroupSizeY << "," << maxGroupSizeZ << ")\n";

    // compute shader
    ComputeShader testCompute(afs::root("src/shaders/basic_compute_shader.cs"), wd.screenWidth, wd.screenHeight);
    testCompute.execute();

    // load some random mesh
    Model robot(afs::root("resources/models/cyborg/cyborg.obj"));
    Model trophy(afs::root("resources/models/sample2.obj"));
    Model unitCube(afs::root("resources/models/unit_cube.obj"));

    // configure depth map FBO
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // create depth cubemap texture
    unsigned int depthCubemap;
    glGenTextures(1, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0,
                     GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // shader configuration
    // --------------------
    colorShader.use();
    colorShader.setInt("diffuseTexture", 0);
    colorShader.setInt("depthMap", 1);

    vec3 lightPos(10.0f, 10.0f, 0.0f);
    bool shadows = true;
    bool shadowKeyPressed = false;

    Gizmo gizmo;
    LineRenderer lineRenderer;
    TextureRenderer textureRenderer;

    float deltaTime, lastFrame = glfwGetTime();
    while (!glfwWindowShouldClose(window.get())) {
        // per-frame time logic
        // --------------------
        float currentFrame = (float) (glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window.get(), deltaTime, camera, shadows, shadowKeyPressed);

        // move light position over time
        lightPos.z = static_cast<float>(sin(glfwGetTime() * 0.5) * 3.0);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 0. create depth cubemap transformation matrices
        // -----------------------------------------------
        float near_plane = 1.0f;
        float far_plane = 25.0f;
        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float) SHADOW_WIDTH / (float) SHADOW_HEIGHT,
                                                near_plane, far_plane);
        std::vector<glm::mat4> shadowTransforms;
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f),
                                                            glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f),
                                                            glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f),
                                                            glm::vec3(0.0f, 0.0f, 1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f),
                                                            glm::vec3(0.0f, 0.0f, -1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f),
                                                            glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f),
                                                            glm::vec3(0.0f, -1.0f, 0.0f)));

        // 1. render scene to depth cubemap
        // --------------------------------
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        linearDepthShader.use();
        for (unsigned int i = 0; i < 6; ++i)
            linearDepthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
        linearDepthShader.setFloat("far_plane", far_plane);
        linearDepthShader.setVec3("lightPos", lightPos);
        // renderScene(linearDepthShader, objects);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. render scene as normal
        glViewport(0, 0, wd.screenWidth, wd.screenHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        colorShader.use();
        glm::mat4 projection = camera.GetProjectionMatrix(wd.screenWidth, wd.screenHeight);
        glm::mat4 view = camera.GetViewMatrix();
        colorShader.setMat4("projection", projection);
        colorShader.setMat4("view", view);
        // set lighting uniforms
        colorShader.setVec3("lightPos", lightPos);
        colorShader.setVec3("viewPos", camera.Position);
        colorShader.setInt("shadows", shadows ? 1 : 0); // enable/disable shadows by pressing 'SPACE'
        colorShader.setFloat("far_plane", far_plane);
        // renderScene(colorShader, objects);

        gizmo.render(camera, lightPos, vec2(wd.screenWidth, wd.screenHeight));

        // lineRenderer.queueBox(objects[1].transform, objects[1].model->meshes[0].boundingBox);
        // lineRenderer.queueBox(Transform{}, trophySdf.bb, vec3(0.0, 1.0, 0.0));
        // lineRenderer.queueBox(Transform{}, trophySdf.outerBB, vec3(1.0, 0.0, 0.0));
        // trophySdf.loopOverCubes([&](int i, int j, int k, BoundingBox bb) {
        //     vec3 color;
        //     if(trophySdf.distances[i][j][k] < 0) {
        //         lineRenderer.queueBox(Transform{}, bb, vec3(0.0, 1.0, 0.0));
        //         // color = vec3(1.0, 0.0, 0.0); // RED
        //     } else {
        //     }
        // });
        // // lineRenderer.queueLine(raymarchDebugRay, WHITE);
        // int i = 0;
        // vec3 colors[3] = {vec3(1.0,0.0,0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0)};
        // for (auto &p : trophySdf.isectPoints) {
        //     auto [a,b,c] = p;
        //     lineRenderer.queueBox(Transform{.translation = a}, BoundingBox(vec3(-0.1 + i * 0.01), vec3(0.1 + i * 0.01)), colors[i % 3] );
        //     lineRenderer.queueBox(Transform{.translation = b}, BoundingBox(vec3(-0.1 + i * 0.01), vec3(0.1 + i * 0.01)), colors[i% 3] );
        //     lineRenderer.queueBox(Transform{.translation = c}, BoundingBox(vec3(-0.1 + i * 0.01), vec3(0.1 + i * 0.01)), colors[i% 3] );
        //     i++;
        // }
        // lineRenderer.queueBox(Transform{.translation = trophySdf.facePoint},
        //     BoundingBox(vec3(-0.1 + i * 0.01), vec3(0.1 + i * 0.01)) );

        // show tri normals
        // for (auto [start, end] : trophySdf.faceNormals) {
        //     lineRenderer.queueLine(start, end);
        // }

        lineRenderer.render(projection, view);

        // if(showRaymarchResult) {
        //     textureRenderer.render(raymarchResult);
        // }
        testCompute.execute();
        textureRenderer.renderRaw(testCompute.textureId);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window.get());
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

vector<vec4> raymarch(vector<Ray> &rays, SdfModel &sdf) {
    vector<vec4> color(rays.size(), vec4());
    for(int i = 0; i < rays.size(); ++i) {
        if(sdf.findHitPositions(rays[i], nullptr)) {
            color[i]=(vec4(1.0, 1.0, 1.0, 0.0));
        }
    }
    return color;
}

void renderScene(Shader &shader, vector<Object> &objects) {
    glEnable(GL_CULL_FACE);

    for (auto &object: objects) {
        if (object.shouldRender) {
            shader.setMat4("model", object.transform.getModelMatrix());
            shader.setVec4("diffuseColor", object.color);
            object.model->draw(shader);
        }
    }
}

void processInput(GLFWwindow *window, float deltaTime, Camera &camera, bool &shadows, bool &shadowsKeyPressed) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboardFPS(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboardFPS(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboardFPS(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboardFPS(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
        camera.ProcessKeyboardArcball(true);
    else if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_RELEASE)
        camera.ProcessKeyboardArcball(false);


    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !shadowsKeyPressed) {
        shadows = !shadows;
        shadowsKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
        shadowsKeyPressed = false;
    }
}
