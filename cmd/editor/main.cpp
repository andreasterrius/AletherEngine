#include <iostream>
#include <memory>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include <src/renderer/line_renderer.h>
#include <src/window.h>
#include <src/data/shader.h>
#include <src/file_system.h>
#include <src/util.h>
#include <src/camera.h>
#include <src/data/ray.h>
#include <src/data/boundingbox.h>
#include "src/data/model.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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
    cout << r.toString() << "\n";

    return r;
}


void renderScene(Shader &shader, Model &object);

void renderCube();

void renderBoundingBoxWireframe(LineRenderer &lr, BoundingBox bb);

void processInput(GLFWwindow *window, float deltaTime, Camera &camera, bool &shadows, bool &shadowsKeyPressed);

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        //getting cursor position
        glfwGetCursorPos(window, &xpos, &ypos);
        cout << "Cursor Position at (" << xpos << " : " << ypos << endl;
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

    int screenWidth = 1024;
    int screenHeight = 768;
    Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

    glfwInit();
    auto window = createWindow(screenWidth, screenHeight);
    if (window == nullptr) {
        glfwTerminate();
        cerr << "window not found";
        return -1;
    }

    WindowData wd{
            .camera = &camera,
            .firstMouse = true
    };
    glfwSetMouseButtonCallback(window.get(), mouseButtonCallback);
    glfwSetCursorPosCallback(window.get(), mouseCallback);
    glfwSetWindowUserPointer(window.get(), &wd);

    Shader colorShader(afs::root("src/shaders/point_shadows.vs").c_str(),
                       afs::root("src/shaders/point_shadows.fs").c_str());
    Shader linearDepthShader(afs::root("src/shaders/point_shadows_depth.vs").c_str(),
                             afs::root("src/shaders/point_shadows_depth.fs").c_str(),
                             afs::root("src/shaders/point_shadows_depth.gs").c_str());

    auto woodTextureOpt = Util::loadTexture(afs::root("resources/textures/wood.png").c_str());
    if (!woodTextureOpt.has_value()) {
        cerr << "wood texture not found";
        return -2;
    }
    unsigned int woodTexture = woodTextureOpt.value();

    // load some random mesh
    Model model(afs::root("resources/models/Barrel_A.obj"));

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

    vec3 lightPos(0.0f, 0.0f, 0.0f);
    bool shadows = true;
    bool shadowKeyPressed = false;

    LineRenderer lineRenderer;
    Ray lastMouseRay(vec3(0.0), camera.Front);

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
        renderScene(linearDepthShader, model);

        model.draw(linearDepthShader);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. render scene as normal
        //         glViewport(0, 0, screenWidth, screenHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        colorShader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float) screenWidth / (float) screenHeight,
                                                0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        colorShader.setMat4("projection", projection);
        colorShader.setMat4("view", view);
        // set lighting uniforms
        colorShader.setVec3("lightPos", lightPos);
        colorShader.setVec3("viewPos", camera.Position);
        colorShader.setInt("shadows", shadows ? 1 : 0); // enable/disable shadows by pressing 'SPACE'
        colorShader.setFloat("far_plane", far_plane);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
        renderScene(colorShader, model);

        if (glfwGetMouseButton(window.get(), GLFW_MOUSE_BUTTON_LEFT)) {
            double mouseX, mouseY;
            glfwGetCursorPos(window.get(), &mouseX, &mouseY);
            lastMouseRay = getMouseRay(mouseX, mouseY, screenWidth, screenHeight,
                                       projection, camera.GetViewMatrix());
        }

        lineRenderer.queue(lastMouseRay);
//        renderBoundingBoxWireframe(lineRenderer, model.meshes[0].boundingBox);
        lineRenderer.render(projection, view);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window.get());
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void renderScene(Shader &shader, Model &object) {
    // room cube
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(5.0f));
    shader.setMat4("model", model);
//    glDisable(
//            GL_CULL_FACE); // note that we disable culling here since we render 'inside' the cube instead of the usual 'outside' which throws off the normal culling methods.
//    shader.setInt("reverse_normals",
//                  1); // A small little hack to invert normals when drawing cube from the inside so lighting still works.
//    renderCube();
    shader.setInt("reverse_normals", 0); // and of course disable it
    glEnable(GL_CULL_FACE);
    // cubes
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(4.0f, -3.5f, 0.0));
    model = glm::scale(model, glm::vec3(0.5f));
    shader.setMat4("model", model);
    renderCube();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.0f, 3.0f, 1.0));
    model = glm::scale(model, glm::vec3(0.75f));
    shader.setMat4("model", model);
    renderCube();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-3.0f, -1.0f, 0.0));
    model = glm::scale(model, glm::vec3(0.5f));
    shader.setMat4("model", model);
    renderCube();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.5f, 1.0f, 1.5));
    model = glm::scale(model, glm::vec3(0.5f));
    shader.setMat4("model", model);
    renderCube();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.5f, 2.0f, -3.0));
    model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    model = glm::scale(model, glm::vec3(0.75f));
    shader.setMat4("model", model);
    renderCube();

    mat4 modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat, glm::vec3(4.0f, -3.5f, 0.0));
    modelMat = glm::scale(modelMat, glm::vec3(1.0f));
    shader.setMat4("model", modelMat);
    object.draw(shader);
}

// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;

void renderCube() {
    // initialize (if necessary)
    if (cubeVAO == 0) {
        float vertices[] = {
                // back face
                -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
                1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
                -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // top-left
                // front face
                -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
                1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // bottom-right
                1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
                1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
                -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // top-left
                -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
                // left face
                -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
                -1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
                -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-right
                -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
                // right face
                1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
                1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
                1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
                1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
                1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left
                // bottom face
                -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // top-left
                1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
                1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
                -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
                -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
                // top face
                -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
                1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
                1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top-right
                1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
                -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
                -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f  // bottom-left
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void processInput(GLFWwindow *window, float deltaTime, Camera &camera, bool &shadows, bool &shadowsKeyPressed) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !shadowsKeyPressed) {
        shadows = !shadows;
        shadowsKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
        shadowsKeyPressed = false;
    }
}

void renderBoundingBoxWireframe(LineRenderer &lr, BoundingBox bb) {
    lr.queue(vec3(bb.min.x, bb.min.y, bb.min.z), vec3(bb.min.x, bb.min.z, bb.max.z));
    lr.queue(vec3(bb.min.x, bb.min.y, bb.min.z), vec3(bb.min.x, bb.max.z, bb.min.z));
    lr.queue(vec3(bb.min.x, bb.min.y, bb.min.z), vec3(bb.max.x, bb.min.z, bb.min.z));

    lr.queue(vec3(bb.max.x, bb.max.y, bb.max.z), vec3(bb.min.x, bb.max.z, bb.max.z));
    lr.queue(vec3(bb.max.x, bb.max.y, bb.max.z), vec3(bb.max.x, bb.min.z, bb.max.z));
    lr.queue(vec3(bb.max.x, bb.max.y, bb.max.z), vec3(bb.max.x, bb.max.z, bb.min.z));
}


