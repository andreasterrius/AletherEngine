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
class Raymarcher {
    unsigned int vao, vbo;
    Shader shader;

public:
    int screenWidth;
    int screenHeight;

    Raymarcher(int screenWidth, int screenHeight) : screenWidth(screenWidth), screenHeight(screenHeight),
                                                    shader(afs::root("src/shaders/raymarch.vert").c_str(),
                                                           afs::root("src/shaders/raymarch.frag").c_str()) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        const static GLfloat vertices[] = {
            -1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f, 1.0f
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }

    void draw(Camera& camera, SdfModel& sdfModel) {
        glDisable(GL_CULL_FACE);

        shader.use();
        shader.setFloat("iTime", glfwGetTime());
        shader.setVec2("iResolution", vec2(screenWidth, screenHeight));
        shader.setVec3("cameraPos", camera.Position);
        mat4 invViewProj = inverse(camera.GetProjectionMatrix(screenWidth, screenHeight) * camera.GetViewMatrix());
        shader.setMat4("invViewProj", invViewProj);

        sdfModel.bindToShader(shader);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glEnable(GL_CULL_FACE);
    }
};

// should hold non owning datas
struct WindowData {
    Raymarcher* raymarcher;
    Camera* camera;
    bool firstMouse;
    float lastX;
    float lastY;

    int screenWidth;
    int screenHeight;
    bool isCursorDisabled;
};

void processInput(GLFWwindow* window, float deltaTime, Camera& camera, bool& shadows, bool& shadowsKeyPressed);

int main() {
    int windowWidth = 400;
    int windowHeight = 800;
    Camera camera(ARCBALL, glm::vec3(3.0f, 3.0f, 10.0f));

    glfwInit();
    auto window = Window(windowWidth, windowHeight, "Raymarch SDF");
    window.set_debug(true);
    window.set_default_inputs(DefaultInputs{
        .keyboard_key_to_disable_cursor = GLFW_KEY_L,
        .keyboard_key_to_enable_cursor = GLFW_KEY_L
    });
    window.attach_mouse_button_callback([](int button, int action, int mods) {});
    window.attach_cursor_pos_callback([&](double xpos, double ypos, double xoffset, double yoffset) {
        camera.ProcessMouseMovement(xoffset, yoffset);
    });

    Raymarcher raymarcher(windowWidth, windowHeight);
    window.attach_framebuffer_size_callback([&](int width, int height) {
        raymarcher.screenWidth = width;
        raymarcher.screenHeight = height;
    });
    window.attach_scroll_callback([&](double xoffset, double yoffset) {
        camera.ProcessMouseScroll(yoffset);
    });

    Shader colorShader(afs::root("src/shaders/point_shadows.vs").c_str(),
                       afs::root("src/shaders/point_shadows.fs").c_str());
    // load some random mesh
    Model trophy(afs::root("resources/models/sample.obj"));

    // shader configuration
    // --------------------
    colorShader.use();
    colorShader.setInt("diffuseTexture", 0);
    colorShader.setInt("depthMap", 1);

    vec3 lightPos(10.0f, 10.0f, 0.0f);
    bool shadows = true;
    bool shadowKeyPressed = false;

    LineRenderer lineRenderer;

    vector<Object> objects{
        Object{
            .transform = Transform{
                .translation = vec3(0.0f),
            },
            .model = make_shared<Model>(std::move(trophy))
        },
    };

    SdfModel trophySdf(*objects[0].model.get(), 64);
    trophySdf.writeToFile("resources/trophySdf.txt");

    float deltaTime, lastFrame = glfwGetTime();
    while (!glfwWindowShouldClose(window.get())){
        // per-frame time logic
        // --------------------
        float currentFrame = (float)(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window.get(), deltaTime, camera, shadows, shadowKeyPressed);

        // move light position over time
        lightPos.z = static_cast<float>(sin(glfwGetTime() * 0.5) * 3.0);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float near_plane = 1.0f;
        float far_plane = 25.0f;;

        // 2. render scene as normal
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //raymarcher.draw(camera);
        trophySdf.loopOverCubes([&](int i, int j, int k, BoundingBox bb) {
            lineRenderer.queueBox(Transform{}, bb);
        });

        raymarcher.draw(camera, trophySdf);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window.get());
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window, float deltaTime, Camera& camera, bool& shadows, bool& shadowsKeyPressed) {
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


    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !shadowsKeyPressed){
        shadows = !shadows;
        shadowsKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE){
        shadowsKeyPressed = false;
    }
}
