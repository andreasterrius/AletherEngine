#include <src/camera.h>
#include <src/data/boundingbox.h>
#include <src/data/model.h>
#include <src/data/object.h>
#include <src/data/ray.h>
#include <src/data/shader.h>
#include <src/data/transform.h>
#include <src/file_system.h>
#include <src/gizmo/gizmo.h>
#include <src/renderer/line_renderer.h>
#include <src/sdf_model.h>
#include <src/sdf_model_packed.h>
#include <src/util.h>
#include <src/window.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>

using namespace std;
using namespace glm;
using namespace ale;

using afs = ale::FileSystem;

class Raymarcher {
  unsigned int vao, vbo;
  unsigned int packed_ssbo = 0;
  Shader shader;

 public:
  int screenWidth;
  int screenHeight;

  Raymarcher(int screenWidth, int screenHeight, string vertexPath,
             string fragmentPath)
      : screenWidth(screenWidth),
        screenHeight(screenHeight),
        shader(vertexPath.c_str(), fragmentPath.c_str()) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    const static GLfloat vertices[] = {-1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  -1.0f,
                                       1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f};
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);
  }

  Raymarcher(int screenWidth, int screenHeight)
      : Raymarcher(screenWidth, screenHeight,
                   afs::root("src/shaders/raymarch.vert"),
                   afs::root("src/shaders/raymarch.frag")) {
    // empty
  }

  void draw(Camera &camera, SdfModel &sdfModel, Transform transform) {
    glDisable(GL_CULL_FACE);

    shader.use();
    shader.setFloat("iTime", glfwGetTime());
    shader.setVec2("iResolution", vec2(screenWidth, screenHeight));
    shader.setVec3("cameraPos", camera.Position);
    mat4 invViewProj =
        inverse(camera.GetProjectionMatrix(screenWidth, screenHeight) *
                camera.GetViewMatrix());
    shader.setMat4("invViewProj", invViewProj);
    shader.setMat4("modelMat", transform.getModelMatrix());
    shader.setMat4("invModelMat", inverse(transform.getModelMatrix()));

    sdfModel.bindToShader(shader);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glEnable(GL_CULL_FACE);
  }

  struct PackedSdfOffsetDetail {
    alignas(16) mat4 modelMat;
    alignas(16) mat4 invModelMat;
    alignas(16) vec4 innerBBMin;
    alignas(16) vec4 innerBBMax;
    alignas(16) vec4 outerBBMin;
    alignas(16) vec4 outerBBMax;
    int atlasIndex;
    int atlasCount;
    int _1 = 0;  // padding
    int _2 = 0;  // padding
  };

  void draw_packed(Camera &camera, SdfModelPacked &sdfModelPacked,
                   Transform transform) {
    glDisable(GL_CULL_FACE);

    if (packed_ssbo == 0) {
      // TODO : Refactor this on creation, not on render
      vector<PackedSdfOffsetDetail> details;
      for (auto &p : sdfModelPacked.get_offsets()) {
        details.push_back(PackedSdfOffsetDetail{
            .modelMat = glm::mat4(1.0),
            .invModelMat = glm::mat4(1.0),
            .innerBBMin = vec4(p.inner_bb.min, 0.0),
            .innerBBMax = vec4(p.inner_bb.max, 0.0),
            .outerBBMin = vec4(p.outer_bb.min, 0.0),
            .outerBBMax = vec4(p.outer_bb.max, 0.0),
            .atlasIndex = p.atlas_index,
            .atlasCount = p.atlas_count,
            ._1 = 0,
            ._2 = 0,
        });
      }
      int details_size = details.size();

      cout << "stride: " << sizeof(PackedSdfOffsetDetail) << std::endl;

      // ssbo for packed sdf
      glGenBuffers(1, &packed_ssbo);
      glBindBuffer(GL_SHADER_STORAGE_BUFFER, packed_ssbo);
      glBufferData(
          GL_SHADER_STORAGE_BUFFER,
          sizeof(unsigned int) * 4 + sizeof(PackedSdfOffsetDetail) *
                                         sdfModelPacked.get_offsets().size(),
          nullptr, GL_STATIC_DRAW);
      glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int) * 4,
                      &details_size);  // pass size
      glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int) * 4,
                      details.size() * sizeof(PackedSdfOffsetDetail),
                      details.data());
    }
    // bind ssbo
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, this->packed_ssbo);

    shader.use();
    shader.setFloat("iTime", glfwGetTime());
    shader.setVec2("iResolution", vec2(screenWidth, screenHeight));
    shader.setVec3("cameraPos", camera.Position);
    mat4 invViewProj =
        inverse(camera.GetProjectionMatrix(screenWidth, screenHeight) *
                camera.GetViewMatrix());
    shader.setMat4("invViewProj", invViewProj);

    // binds texture2D atlas[16];
    // binds int atlasSize;
    sdfModelPacked.bind_to_shader(shader);

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
  Raymarcher *raymarcher;
  Camera *camera;
  bool firstMouse;
  float lastX;
  float lastY;

  int screenWidth;
  int screenHeight;
  bool isCursorDisabled;
};

void processInput(GLFWwindow *window, float deltaTime, Camera &camera);

int main() {
  int windowWidth = 800;
  int windowHeight = 800;
  Camera camera(ARCBALL, glm::vec3(3.0f, 3.0f, 7.0f));

  glfwInit();
  auto window = Window(windowWidth, windowHeight, "Raymarch SDF");
  window.set_debug(false);
  window.set_default_inputs(
      DefaultInputs{.keyboard_key_to_disable_cursor = GLFW_KEY_L,
                    .keyboard_key_to_enable_cursor = GLFW_KEY_L});
  window.attach_mouse_button_callback([](int button, int action, int mods) {});
  window.attach_cursor_pos_callback(
      [&](double xpos, double ypos, double xoffset, double yoffset) {
        camera.ProcessMouseMovement(xoffset, yoffset);
      });

  Raymarcher raymarcher(windowWidth, windowHeight);
  Raymarcher raymarcher2d(windowWidth, windowHeight,
                          afs::root("src/shaders/raymarch.vert"),
                          afs::root("src/shaders/raymarch_atlas.frag"));

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
  Model monkey(afs::root("resources/models/monkey.obj"));
  Model unitCube(afs::root("resources/models/unit_cube.obj"));

  LineRenderer lineRenderer;

  // SdfModel trophySdf(*objects[0].model.get(), 16);
  SdfModel monkeySdfGpu64(monkey, Texture3D::load("monkey64"), 64);
  SdfModel monkeySdfGpu32(monkey, Texture3D::load("monkey32"), 32);
  SdfModel monkeySdfGpu16(monkey, Texture3D::load("monkey16"), 16);

  SdfModel unitCubeSdfGpu32(unitCube, Texture3D::load("unit_cube32"), 32);

  SdfModelPacked monkeySdfPacked(vector<SdfModel *>{&monkeySdfGpu64});

  Transform transform;
  transform.translation = vec3(0.0, 0.0, 0.0);

  glm::vec3 eulerAngles = glm::vec3(glm::radians(45.0f), glm::radians(0.0f),
                                    glm::radians(0.0f));  // Pitch, yaw, roll
  transform.rotation = quat(eulerAngles);

  int sdfModel = 1;

  float deltaTime, lastFrame = glfwGetTime();
  while (!glfwWindowShouldClose(window.get())) {
    // per-frame time logic
    // --------------------
    float currentFrame = (float)(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // input
    // -----
    processInput(window.get(), deltaTime, camera);

    // 1 to check CPU SDF
    // 2 to check GPU SDF
    if (glfwGetKey(window.get(), GLFW_KEY_1) == GLFW_PRESS) {
      sdfModel = 1;
      cout << "sdf 1 enabled\n";
    } else if (glfwGetKey(window.get(), GLFW_KEY_2) == GLFW_PRESS) {
      sdfModel = 2;
      cout << "sdf 2 enabled\n";
    } else if (glfwGetKey(window.get(), GLFW_KEY_3) == GLFW_PRESS) {
      sdfModel = 3;
      cout << "sdf 3 enabled\n";
    }

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (sdfModel == 1) {
      raymarcher.draw(camera, monkeySdfGpu32, Transform{});
    } else if (sdfModel == 2) {
      raymarcher2d.draw_packed(camera, monkeySdfPacked, Transform{});
    }

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved
    // etc.)
    glfwSwapBuffers(window.get());
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}

void processInput(GLFWwindow *window, float deltaTime, Camera &camera) {
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
}
