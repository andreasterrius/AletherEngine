//
// Created by Alether on 3/2/2025.
//

#ifndef LIGHT_H
#define LIGHT_H

#include "src/data/serde/glm.h"

struct AmbientLight {
  float intensity;
  glm::vec3 color;
  glm::vec3 background_color;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AmbientLight, intensity, color,
                                   background_color);

struct Light {
  glm::vec3 color = glm::vec3(1.0f);
  float radius = 1.0f;
  glm::vec3 attenuation = glm::vec3(1.0f, 0.09f, 0.032f);
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Light, color, radius, attenuation);


#endif // LIGHT_H
