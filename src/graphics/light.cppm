//
// Created by Alether on 3/2/2025.
//

module;

#include <glm/glm.hpp>
export module light;

export namespace ale::graphics {

struct AmbientLight {
  float intensity;
  glm::vec3 color;
  glm::vec3 background_color;
};

struct Light {
  glm::vec3 color = glm::vec3(1.0f);
  float radius = 1.0f;
  glm::vec3 attenuation = glm::vec3(1.0f, 0.09f, 0.032f);
};

} // namespace ale::graphics
