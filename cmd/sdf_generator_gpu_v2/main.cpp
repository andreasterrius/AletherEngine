// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <entt/entt.hpp>
#include "src/graphics/sdf/sdf_generator_gpu_v2_shared.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

import color;
import raymarcher_cpu;
import material;
import texture;
import data:stash;
import line_renderer;
import file_system;
import window;
import camera;
import model;
import mesh;
import sdf_generator_gpu;
import sdf_generator_gpu_v2;
import sdf_model;
import basic_renderer;
import line_renderer;
import light;
import static_mesh;
import transform;
import basic_renderer;
import bounding_box;

using namespace ale;
using namespace ale::data;
using namespace ale::graphics;
using namespace ale::graphics::renderer;
using namespace ale::graphics::sdf;
using namespace std;
using namespace glm;


entt::registry new_world(StaticMeshLoader &sm_loader) {
  // Create world
  auto world = entt::registry{};

  // Lights
  {
    const auto entity = world.create();
    world.emplace<Transform>(entity,
                             Transform{.translation = vec3(10.0, 10.0, 10.0)});
    world.emplace<Light>(entity, Light{});

    auto sphere = *sm_loader.get_static_mesh(SM_UNIT_SPHERE);
    sphere.set_cast_shadow(false);
    world.emplace<StaticMesh>(entity, sphere);
  }
  return world;
}

int main() {
  glfwInit();

  auto window = Window(1024, 768, "SDF Generator V2");
  auto camera = Camera(ARCBALL, 1024, 768, glm::vec3(3.0f, 5.0f, -7.0f));
  auto basic_renderer = BasicRenderer();
  auto texture_stash = make_shared<Stash<Texture>>();
  auto sm_loader = StaticMeshLoader(texture_stash);
  auto static_mesh = sm_loader.load_static_mesh(
      afs::root("resources/models/content_browser/tree.obj"));
  auto model = static_mesh.get_model();

  window.set_debug(true);

  auto world = new_world(sm_loader);
  {
    const auto entity = world.create();
    world.emplace<Transform>(entity, Transform{.translation = vec3()});
    world.emplace<StaticMesh>(entity, static_mesh);
    world.emplace<BasicMaterial>(entity, BasicMaterial{});
  }

  SdfGeneratorGPUV2 sdfgen;
  LineRenderer line_renderer;

  int resolution = 16;
  auto texture = std::move(sdfgen.generate_gpu(*model, resolution).at(0));
  auto texture_data = texture.retrieve_data_from_gpu();
  auto sdf_mesh = SdfModel(model->meshes.at(0), std::move(texture), resolution);

  window.attach_cursor_pos_callback(
      [&](double xpos, double ypos, double xoffset, double yoffset) {
        camera.ProcessMouseMovement(xoffset, yoffset);
      });
  window.attach_scroll_callback([&](double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
  });

  vec3 closest_normal = vec3();
  vec3 closest_point = vec3();
  int selected_index = 368;
  int sx, sy, sz = 0;
  vector image(
      resolution,
      vector(resolution, vector(resolution, vec4(0.0f, 0.0f, 0.0f, 0.0f))));

  auto debug_raymarch = false;
  auto cursor_pos = window.get_cursor_pos();
  auto raymarcher_cpu = RaymarcherCpu();

  window.attach_key_callback([&](int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
      if (mods == GLFW_MOD_SHIFT) {
        selected_index += 64;
      } else {
        selected_index += 1;
      }
      if (selected_index >= 8 * 8 * 8)
        selected_index = 0;
    } else if (key == GLFW_KEY_M && action == GLFW_PRESS) {
      if (selected_index > 0)
        selected_index -= 1;
    } else if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
      auto &m = model->meshes[0];
      auto outer_bb = m.boundingBox.apply_scale(Transform{.scale = vec3(1.1f)});
      generate_sdf(ivec3(sx, sy, sz), m.vertices.size(), m.indices.size(),
                   outer_bb.min, outer_bb.max, ivec3(8), m.vertices, m.indices,
                   image, closest_normal);
      auto result = image[sx][sy][sz];
      closest_point = vec3(result[1], result[2], result[3]);
    }

    if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
      debug_raymarch = true;
    } else if (key == GLFW_KEY_UP && action == GLFW_RELEASE) {
      debug_raymarch = false;
    }
  });

  while (!window.get_should_close()) {
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    basic_renderer.render(camera, world);

    line_renderer.queue_box(Transform{}, model->meshes[0].boundingBox, RED);
    line_renderer.queue_box(Transform{}, model->meshes[1].boundingBox, GREEN);
    sdf_mesh.loopOverCubes([&](int x, int y, int z, BoundingBox bb) {
      int index = sdf_mesh.texture3D->get_index(x, y, z, 0);
      float distance = texture_data.at(index);
      vec3 color;
      if (distance > 0.0f) {
        color = WHITE;
      } else {
        color = RED;
      }
      if (selected_index == index) {
        color = GREEN;
        sx = x;
        sy = y;
        sz = z;
        auto center_dir = normalize(bb.getCenter() - closest_point);
        auto d = dot(closest_normal, normalize(bb.getCenter() - closest_point));
        // cout << format("dot:{} | {} {} {} | {} {} {}", d,
        closest_normal.x,
            //                closest_normal.y, closest_normal.z, center_dir.x,
            //                center_dir.y, center_dir.z)
            //      << endl;
            line_renderer.queue_line(closest_point, closest_point + center_dir,
                                     YELLOW);
        line_renderer.queue_line(closest_point, closest_point + closest_normal,
                                 BLUE);
        line_renderer.queue_unit_cube(Transform{
            .translation = normalize(closest_point + closest_normal)});
      }
      if (color == WHITE) {
      } else {
        line_renderer.queue_box(Transform{}, bb, color);
      }
    });
    line_renderer.queue_unit_cube(Transform{.translation = closest_point});
    line_renderer.render(camera.get_projection_matrix(),
                         camera.get_view_matrix());

    if (debug_raymarch) {
      raymarcher_cpu.shoot_ray(window, camera, sdf_mesh);
    }
    raymarcher_cpu.render(camera);

    if (glfwGetKey(window.get(), GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
      camera.ProcessKeyboardArcball(true);
    else if (glfwGetKey(window.get(), GLFW_KEY_LEFT_ALT) == GLFW_RELEASE)
      camera.ProcessKeyboardArcball(false);

    // texture_renderer.render(sdfgen.debug_result.at("monkey16"));
    window.swap_buffer_and_poll_inputs();
  }

  glfwTerminate();
  return 0;
}
