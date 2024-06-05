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

/***
 * Let's try to do raymarching here
 * Vertex Shader just NDC
*  const static GLfloat vertices[] = {
        -1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,
        1.0f,  -1.0f, -1.0f, -1.0f, -1.0f,  1.0f};

        https://github.com/gitEllE-if/ray_marcher/blob/master/srcs/main.c

    1. Setup shader
    2. Render a cute sphere (https://timcoster.com/2020/02/11/raymarching-shader-pt1-glsl/)
    3. Pass 3d texture to shader
    4. Plug in the texture distance function
 */

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


void renderScene(Shader &shader, vector<Object> &objects);

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
    glfwSetScrollCallback(window.get(), scrollCallback);

    Shader colorShader(afs::root("src/shaders/point_shadows.vs").c_str(),
                       afs::root("src/shaders/point_shadows.fs").c_str());

    // load some random mesh
    Model robot(afs::root("resources/models/cyborg/cyborg.obj"));
    Model randomCubes(afs::root("resources/models/sphere_random.obj"));

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

    Object *selectedObject = nullptr;
    vector<Object> objects{
        Object{
            .transform = Transform{
                .translation = vec3(10.0f)
            },
            .model = make_shared<Model>(std::move(ModelFactory::createCubeModel())),
            .shouldRender = false
        },
        Object{
            .transform = Transform{
                .translation = vec3(0.0f),
            },
            .model = make_shared<Model>(std::move(randomCubes))
        },
        Object{
            .transform = Transform{
                .translation = vec3(10.0f),
            },
            .model = make_shared<Model>(std::move(robot)),
            .shouldRender = false
        },
    };
    //SdfModel robotSdf(robot, 4);
    SdfModel randomCubesSdf(*objects[1].model.get(), 8);

    // Debug sphere
    // objects.push_back(Object{
    //     .transform = Transform {
    //         .translation = randomCubesSdf.positions[0][0][0],
    //     },
    //     .model = make_shared<Model>(std::move(ModelFactory::createSphereModel(randomCubesSdf.distances[0][0][0]))),
    //     .color = vec4(1.0, 1.0, 1.0, 0.5),
    // });

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
        float far_plane = 25.0f;;

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
        renderScene(colorShader, objects);

        lineRenderer.queueBox(objects[1].transform, objects[1].model->meshes[0].boundingBox);
        randomCubesSdf.loopOverCubes([&](int i, int j, int k, BoundingBox bb) {
            lineRenderer.queueBox(Transform{}, bb);
        });
        lineRenderer.render(projection, view);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window.get());
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void renderScene(Shader &shader, vector<Object> &objects) {
    // room cube
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(5.0f));
    shader.setMat4("model", model);
    glEnable(GL_CULL_FACE);

    for (auto &object: objects) {
        if(object.shouldRender) {
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

