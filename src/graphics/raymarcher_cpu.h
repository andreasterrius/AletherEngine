//
// Created by Alether on 1/30/2025.
//

#ifndef RAYMARCHER_CPU_H
#define RAYMARCHER_CPU_H

#include "camera.h"
#include "line_renderer.h"
#include "sdf/sdf_model.h"

import window;

namespace ale::graphics {

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
} // namespace ale::graphics

#endif // RAYMARCHER_CPU_H
