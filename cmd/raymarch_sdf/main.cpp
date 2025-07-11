
// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

import transform;
import sdf_model;
import sdf_model_packed;
import window;
import light;
import camera;
import gizmo;
import thumbnail_generator;
import deferred_renderer;
import static_mesh;
import texture;
import line_renderer;
import shader;
import file_system;
import model;

using namespace std;
using namespace ale;
using namespace ale::graphics::renderer;
using namespace ale::graphics;
using namespace ale::graphics::sdf;
using namespace ale::data;
using namespace glm;


class Raymarcher {
  unsigned int vao, vbo;
  unsigned int packed_ssbo = 0;
  Shader shader;

public:
  int screenWidth;
  int screenHeight;

  Raymarcher(int screenWidth, int screenHeight, string vertexPath,
             string fragmentPath) :
      screenWidth(screenWidth),
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
                          (void *) 0);
    glEnableVertexAttribArray(0);
  }

  Raymarcher(int screenWidth, int screenHeight) :
      Raymarcher(screenWidth, screenHeight,
                 afs::root("cmd/raymarch_sdf/raymarch.vert"),
                 afs::root("cmd/raymarch_sdf/raymarch.frag")) {
    // empty
  }

  void draw(Camera &camera, SdfModel &sdfModel, Transform transform) {
    glDisable(GL_CULL_FACE);

    shader.use();
    shader.setFloat("iTime", glfwGetTime());
    shader.setVec2("iResolution", vec2(screenWidth, screenHeight));
    shader.setVec3("cameraPos", camera.Position);
    mat4 invViewProj =
        inverse(camera.get_projection_matrix(screenWidth, screenHeight) *
                camera.get_view_matrix());
    shader.setMat4("invViewProj", invViewProj);
    shader.setMat4("modelMat", transform.get_model_matrix());
    shader.setMat4("invModelMat", inverse(transform.get_model_matrix()));

    sdfModel.bind_to_shader(shader);

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
    int _1 = 0; // padding
    int _2 = 0; // padding
  };

  void draw_packed(Camera &camera, SdfModelPacked &sdfModelPacked,
                   vector<Transform> &transform) {
    glDisable(GL_CULL_FACE);

    if (packed_ssbo == 0) {
      vector<PackedSdfOffsetDetail> details;
      for (int i = 0; i < sdfModelPacked.get_offsets().size(); ++i) {
        auto &p = sdfModelPacked.get_offsets()[i];
        auto &t = transform[i];
        details.push_back(PackedSdfOffsetDetail{
            .modelMat = t.get_model_matrix(),
            .invModelMat = inverse(t.get_model_matrix()),
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

      // ssbo for packed sdf
      glGenBuffers(1, &packed_ssbo);
      glBindBuffer(GL_SHADER_STORAGE_BUFFER, packed_ssbo);
      glBufferData(GL_SHADER_STORAGE_BUFFER,
                   sizeof(unsigned int) * 4 +
                       sizeof(PackedSdfOffsetDetail) *
                           sdfModelPacked.get_offsets().size(),
                   nullptr, GL_STATIC_DRAW);
      glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int) * 4,
                      &details_size); // pass size
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
        inverse(camera.get_projection_matrix(screenWidth, screenHeight) *
                camera.get_view_matrix());
    shader.setMat4("invViewProj", invViewProj);

    // binds texture2D atlas[16];
    // binds int atlasSize;
    vector<pair<Transform, vector<unsigned int>>> entries = {};
    sdfModelPacked.bind_to_shader(shader, entries, 0);

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
  Camera camera(ARCBALL, windowWidth, windowHeight,
                glm::vec3(3.0f, 3.0f, 7.0f));

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

  window.attach_framebuffer_size_callback([&](int width, int height) {
    raymarcher.screenWidth = width;
    raymarcher.screenHeight = height;
  });
  window.attach_scroll_callback([&](double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
  });

  // load some random mesh
  Model monkey(afs::root("resources/models/monkey.obj"));
  Model unitCube(afs::root("resources/models/unit_cube.obj"));

  LineRenderer lineRenderer;

  // SdfModel trophySdf(*objects[0].model.get(), 16);
  SdfModel monkeySdfGpu64(monkey.meshes[0], Texture3D::load("monkey64"), 64);
  SdfModel monkeySdfGpu32(monkey.meshes[0], Texture3D::load("monkey64_b"), 64);
  SdfModel monkeySdfGpu16(monkey.meshes[0], Texture3D::load("monkey16"), 16);

  SdfModel unitCubeSdfGpu64(unitCube.meshes[0], Texture3D::load("unit_cube64"),
                            64);
  SdfModel unitCubeSdfGpu32(unitCube.meshes[0], Texture3D::load("unit_cube32"),
                            64);

  // SdfModelPacked packedSdfs(
  //     vector<SdfModel *>{&monkeySdfGpu64, &unitCubeSdfGpu32});
  // vector<Transform> packedSdfTransforms{
  //     Transform{}, Transform{.translation = vec3(-2.0, 0.0, 0.0)}};

  Transform transform;
  transform.translation = vec3(0.0, 0.0, 0.0);

  glm::vec3 eulerAngles = glm::vec3(glm::radians(45.0f), glm::radians(0.0f),
                                    glm::radians(0.0f)); // Pitch, yaw, roll
  transform.rotation = quat(eulerAngles);

  int sdfModel = 1;

  float deltaTime, lastFrame = glfwGetTime();
  while (!window.get_should_close()) {
    // per-frame time logic
    // --------------------
    float currentFrame = (float) (glfwGetTime());
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
      raymarcher.draw(camera, monkeySdfGpu64, Transform{});
    } else if (sdfModel == 3) {
      // raymarcher2d.draw_packed(camera, packedSdfs, packedSdfTransforms);
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
