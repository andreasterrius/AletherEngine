
// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include "src/config.h"
#include "src/data/compute_shader.h"
#include "src/data/model.h"
#include <glm/glm.hpp>
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include "src/camera.h"

#include <stb_image.h>

using namespace std;
using namespace glm;
using namespace ale;

struct GpuData {
  ivec4 size; // [0] = vertices.size, [1] = indices.size
  vec4 inner_bb_min;
  vec4 inner_bb_max;
  vec4 outer_bb_min;
  vec4 outer_bb_max;
};

struct GPUObject {
  mat4 model_mat;
  mat4 inv_model_mat;
  vec4 inner_bbmin;
  vec4 inner_bbmax;
  vec4 outer_bbmin;
  vec4 outer_bbmax;
};

GLFWwindow *create_window(glm::ivec2 screen_size) {
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  auto window = glfwCreateWindow(screen_size.x, screen_size.y,
                                 "Mesh Distance Field", NULL, NULL);
  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    throw std::runtime_error("loading gl functions failed");
  }

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_FRAMEBUFFER_SRGB);
  return window;
}

unsigned int generate_mdf(Mesh &mesh, int mdf_resolution,
                          ComputeShader &mdf_generator_shader) {
  glUseProgram(mdf_generator_shader.id);

  // Upload all required data for the generator to work
  {
    unsigned int vertex_buffer;
    unsigned int index_buffer;
    unsigned int ubo;

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertex_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 mesh.vertices.size() * sizeof(Vertex), mesh.vertices.data(),
                 GL_STATIC_DRAW);

    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, index_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 mesh.indices.size() * sizeof(unsigned int),
                 mesh.indices.data(), GL_STATIC_DRAW);

    auto outer_bb = mesh.boundingBox.apply_scale(Transform{
        .scale = vec3(1.1, 1.1, 1.1),
    });
    auto gpu_data = GpuData{
        ivec4(mesh.vertices.size(), mesh.indices.size(), 0, 0),
        vec4(mesh.boundingBox.min, 0.0), vec4(mesh.boundingBox.max, 0.0),
        vec4(outer_bb.min, 0.0), vec4(outer_bb.max, 0.0)};

    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GpuData), &gpu_data, GL_STATIC_DRAW);

    // wait until the upload is done
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, vertex_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, index_buffer);
    glBindBufferBase(GL_UNIFORM_BUFFER, 4, ubo);
  }

  // Create a 3D texture to store the generated distance
  unsigned int texture_id;
  {
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_3D, texture_id);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, mdf_resolution, mdf_resolution,
                 mdf_resolution, 0, GL_RGBA, GL_FLOAT,
                 nullptr /*empty texture*/);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_3D, 0);
  }

  // Run the compute shader
  {
    glBindImageTexture(0, texture_id, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32F);
    glDispatchCompute(mdf_resolution, mdf_resolution, mdf_resolution);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT |
                    GL_TEXTURE_UPDATE_BARRIER_BIT);
  }

  return texture_id;
}

void render_scene(unsigned int mdf, Transform &monkey_transform,
                  Shader &render_shader, Mesh &monkey_mesh, Mesh &floor_mesh,
                  Camera &camera) {
  render_shader.use();

  // common information
  render_shader.setVec4("diffuseColor", vec4(1.0, 1.0, 1.0, 0.0));
  render_shader.setMat4("view", camera.GetViewMatrix());
  render_shader.setMat4("projection", camera.GetProjectionMatrix());
  render_shader.setVec3("viewPos", camera.Position);

  // pass the mdf information
  auto inner_bb = monkey_mesh.boundingBox;
  auto outer_bb = monkey_mesh.boundingBox.apply_scale(Transform{
      .scale = vec3(1.1, 1.1, 1.1),
  });
  render_shader.setMat4("mdf.modelMat", monkey_transform.get_model_matrix());
  render_shader.setMat4("mdf.invModelMat",
                        inverse(monkey_transform.get_model_matrix()));
  render_shader.setVec3("mdf.innerBBMin", inner_bb.min);
  render_shader.setVec3("mdf.innerBBMax", inner_bb.max);
  render_shader.setVec3("mdf.outerBBMin", outer_bb.min);
  render_shader.setVec3("mdf.outerBBMax", outer_bb.max);
  render_shader.setVec3("mdf.resolution", vec3(64));

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_3D, mdf);

  // render monkey
  render_shader.setMat4("model", monkey_transform.get_model_matrix());
  monkey_mesh.Draw(render_shader);

  // render floor
  auto floor_transform = Transform{.translation = vec3(0.0f, -5.0f, 0.0f),
                                   .scale = vec3(2.0f, 1.0f, 2.0f)};
  render_shader.setMat4("model", floor_transform.get_model_matrix());
  floor_mesh.Draw(render_shader);
}

int main() {
  glfwInit();

  auto screen_size = glm::ivec2(1920, 1080);
  auto window = create_window(screen_size);
  auto camera = Camera(ARCBALL, screen_size.x, screen_size.y,
                       glm::vec3(6.0f, 10.0f, 14.0f));

  // Model class is similar to LearnOpenGL model class.
  auto monkey_mesh =
      Model(std::string(ALE_ROOT_PATH) + "/resources/models/monkey.obj")
          .meshes.at(0);
  auto floor_mesh =
      Model(std::string(ALE_ROOT_PATH) + "/resources/models/floor_cube.obj")
          .meshes.at(0);

  // Load shaders required
  auto mdf_generator_shader = ComputeShader(
      std::string(ALE_ROOT_PATH) + "/src/renderer/sdf_generator_gpu_v2.cs");
  auto render_shader =
      Shader((std::string(ALE_ROOT_PATH) + "/src/shaders/mdf/scene_renderer.vs")
                 .c_str(),
             (std::string(ALE_ROOT_PATH) + "/src/shaders/mdf/scene_renderer.fs")
                 .c_str());

  auto mdf = generate_mdf(monkey_mesh, 64, mdf_generator_shader);
  auto monkey_transform = Transform{};

  while (!glfwWindowShouldClose(window)) {
    glClearColor(135.0 / 255, 206.0 / 255, 235.0 / 255, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // move the monke
    auto curr_time = glfwGetTime();

    auto rotate = glm::identity<glm::quat>();
    rotate = glm::rotate(rotate, (float)sin(curr_time), vec3(0.0, 1.0, 0.0));
    monkey_transform.rotation = rotate;

    monkey_transform.translation = vec3(sin(curr_time), 0.0, cos(curr_time));
    monkey_transform.scale =
        vec3(abs(sin(curr_time)), abs(sin(curr_time)), abs(sin(curr_time))) +
        vec3(0.5f);

    render_scene(mdf, monkey_transform, render_shader, monkey_mesh, floor_mesh,
                 camera);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
