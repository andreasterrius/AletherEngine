//
// Created by Alether on 1/30/2025.
//

#ifndef RAYMARCHER_CPU_H
#define RAYMARCHER_CPU_H

#include "../sdf_model.h"
#include "../window.h"
#include "line_renderer.h"
#include "src/camera.h"

namespace ale {
// Class to debug raymarch attempts for SDF.
// I think a better way is to combine it with the shader later.
class RaymarcherCpu {
private:
  LineRenderer line_renderer;
  vector<glm::vec3> ray_hit_pos;

public:
  RaymarcherCpu();

  Ray get_mouse_ray(float mouseX, float mouseY, float screenWidth,
                    float screenHeight, glm::mat4 projMat, glm::mat4 viewMat);

  void shoot_ray(Window &window, Camera &camera, SdfModel &sdf_model);

  void render(Camera &camera);
};
} // namespace ale

#endif // RAYMARCHER_CPU_H
