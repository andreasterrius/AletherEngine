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
#include <src/data/model.h>
#include <src/data/object.h>
#include <src/gizmo/gizmo.h>
#include <src/data/transform.h>
#include <src/sdf_model.h>

#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>

using namespace std;
using namespace glm;
using namespace ale;

using afs = ale::FileSystem;

// TODO:
// 1. Create an arcball camera control (DONE)
// 2. Insert a triangle the scene (DONE)
// 3. Generate the SDF cube view for a sphere
    //3.1 generate the points first and visualize them in the viewport
    //3.2 loop over all the triangles that exist in the scene
    //3.3 make the loop parallel for every n triangle (where n is number of thread/spread algorithm)
    //3.4 calculate the distance with barycentry coordinage from each of the triangle to specific points in the 3d array.

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


void renderScene(Shader &shader, Model &robot, vector<Object> &objects);

void renderCube();

//void renderBoundingBoxWireframe(LineRenderer &lr, Transform transform, BoundingBox bbT);

void processInput(GLFWwindow *window, float deltaTime, Camera &camera, bool &shadows, bool &shadowsKeyPressed);

void pickupObject(GLFWwindow *window, WindowData wd, vector<Object> &objectsToSelect, Object*& selectedObject,
                  Camera &camera, Gizmo &gizmo, Ray &lastRay);

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

    Camera camera(ARCBALL, glm::vec3(0.0f, 0.0f, 5.0f));

    WindowData wd{
            .camera = &camera,
            .firstMouse = true,
            .screenWidth = 1024,
            .screenHeight = 768,
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
    Model robot(afs::root("resources/models/cyborg/cyborg.obj"));
    Model randomCubes(afs::root("resources/models/sphere_random.obj"));

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
    Ray lastMouseRay(vec3(0.0), camera.Front);

    Object* selectedObject = nullptr;
    vector<Object> objects{
        Object {
                .transform = Transform{
                        .translation = vec3(1.0f),
                        .scale = vec3(1.0f),
                        .rotation = vec4(1.0f),
                },
                .model = make_shared<Model>(std::move(ModelFactory::createCubeModel()))
        },
        Object {
            .transform = Transform {},
            .model = make_shared<Model>(std::move(randomCubes))
        }
    };
    objects[0].model->meshes[0].textures.push_back(Texture {
        .id=woodTexture,
        .type="texture_diffuse",
    });

    //SdfModel robotSdf(robot, 4);
    SdfModel randomCubesSdf(*objects[1].model.get(), 4);

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

        // pick up object
        // ------
        pickupObject(window.get(), wd, objects, selectedObject, camera, gizmo, lastMouseRay);

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
        renderScene(linearDepthShader, robot, objects);
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
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
        renderScene(colorShader, robot, objects);

        gizmo.render(camera, lightPos, vec2(wd.screenWidth, wd.screenHeight));

        lineRenderer.queueLine(lastMouseRay);
        lineRenderer.queueLine(vec3(0), vec3(100));
//        lineRenderer.queueBB(Transform{}, robot.meshes[0].boundingBox);
//        lineRenderer.queueBB(objects[0].transform, objects[0].model->meshes[0].boundingBox);
        lineRenderer.queueBox(Transform{}, randomCubesSdf.boundingBox);
        randomCubesSdf.loopOverCubes([&](Transform transform, BoundingBox bb){
            lineRenderer.queueBox(transform, bb);
        });
        lineRenderer.render(projection, view);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window.get());
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void renderScene(Shader &shader, Model &robot, vector<Object> &objects) {
    // room cube
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(5.0f));
    shader.setMat4("model", model);
//    glDisable(
//            GL_CULL_FACE); // note that we disable culling here since we render 'inside' the cube instead of the usual 'outside' which throws off the normal culling methods.
//    shader.setInt("reverse_normals",
//                  1); // A small little hack to invert normals when drawing cube from the inside so lighting still works.
//    renderCube();
//    shader.setInt("reverse_normals", 0); // and of course disable it
    glEnable(GL_CULL_FACE);

    model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    robot.draw(shader);

//    shader.setMat4("model", cube.transform.getModelMatrix());
//    cube.model->draw(shader);

    for(auto &object : objects) {
        shader.setMat4("model", object.transform.getModelMatrix());
        object.model->draw(shader);
    }
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

void pickupObject(GLFWwindow *window,
                  WindowData wd,
                  vector<Object> &objectsToSelect,
                  Object*& selectedObject,
                  Camera &camera,
                  Gizmo &gizmo,
                  Ray &lastRay) {

    bool clickedSomething = false;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)) {
        if (selectedObject != nullptr) {
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);
            Ray mouseRay = getMouseRay(mouseX, mouseY, wd.screenWidth, wd.screenHeight,
                                       camera.GetProjectionMatrix(wd.screenWidth, wd.screenHeight),
                                       camera.GetViewMatrix());
            clickedSomething = gizmo.tryHold(&selectedObject->transform, mouseRay, camera);
            lastRay = mouseRay;
        }

        if (!clickedSomething) {
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);
            Ray mouseRay = getMouseRay(mouseX, mouseY, wd.screenWidth, wd.screenHeight,
                                       camera.GetProjectionMatrix(wd.screenWidth, wd.screenHeight),
                                       camera.GetViewMatrix());

            float furthestT = INFINITY;
            for (int i = 0; i < objectsToSelect.size(); ++i) {
                // check if we are clicking anything
                auto isectT = mouseRay.tryIntersect(objectsToSelect[i].transform,
                                                    objectsToSelect[i].model->meshes[0].boundingBox);
                if (isectT.has_value() && isectT < furthestT) {
                    selectedObject = &objectsToSelect[i];
                    furthestT = *isectT;
                    clickedSomething = true;
                    lastRay = mouseRay;
                }
            }
        }

        if (!clickedSomething) {
            selectedObject = nullptr;
            gizmo.hide();
        }
    }

    if (!clickedSomething && !glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)) {
        gizmo.release();
    }
}


