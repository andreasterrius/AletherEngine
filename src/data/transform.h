//
// Created by Alether on 4/26/2024.
//

#ifndef ALETHERENGINE_TRANSFORM_H
#define ALETHERENGINE_TRANSFORM_H

#include "serde/glm.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nlohmann/json.hpp>

using namespace glm;

namespace ale {
class Transform {
public:
  vec3 translation = vec3(0.0f);
  vec3 scale = vec3(1.0f);
  quat rotation = glm::identity<quat>(); // quaternion

  mat4 get_model_matrix();
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Transform, translation, scale, rotation);
} // namespace ale

#endif // ALETHERENGINE_TRANSFORM_H
