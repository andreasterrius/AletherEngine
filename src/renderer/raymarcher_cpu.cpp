//
// Created by Alether on 1/30/2025.
//

#include "raymarcher_cpu.h"

namespace ale {
RaymarcherCpu::RaymarcherCpu() {}

Ray RaymarcherCpu::get_mouse_ray(float mouseX, float mouseY, float screenWidth,
                                 float screenHeight, mat4 projMat,
                                 mat4 viewMat) {
  vec4 rayStartNdc = vec4(((mouseX / screenWidth) * 2) - 1,
                          ((mouseY / screenHeight) * 2) - 1, -1.0f, 1.0f);
  vec4 rayEndNdc = vec4(((mouseX / screenWidth) * 2) - 1,
                        ((mouseY / screenHeight) * 2) - 1, 0.0f, 1.0f);

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

void RaymarcherCpu::shoot_ray(Window &window, Camera &camera,
                              SdfModel &sdf_model) {
  ray_hit_pos.clear();
  auto mouse_pos = window.get_cursor_pos();
  auto window_size = window.get_size();
  Ray ray =
      get_mouse_ray(mouse_pos.x, mouse_pos.y, window_size[0], window_size[1],
                    camera.GetProjectionMatrix(window_size[0], window_size[1]),
                    camera.GetViewMatrix());
  sdf_model.find_hit_positions(ray, &ray_hit_pos);
}

void RaymarcherCpu::render(Camera &camera) {
  for (auto hp : ray_hit_pos) {
    line_renderer.queue_unit_cube(
        Transform{.translation = hp, .scale = vec3(0.1f)});
  }
  line_renderer.render(camera.GetProjectionMatrix(), camera.GetViewMatrix());
}
} // namespace ale
