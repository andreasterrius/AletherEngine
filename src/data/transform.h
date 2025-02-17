//
// Created by Alether on 4/26/2024.
//

#ifndef ALETHERENGINE_TRANSFORM_H
#define ALETHERENGINE_TRANSFORM_H

#include "src/data/serde/glm.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nlohmann/json.hpp>

namespace ale {
class Transform {
public:
  glm::vec3 translation = glm::vec3(0.0f);
  glm::vec3 scale = glm::vec3(1.0f);
  glm::quat rotation = glm::identity<glm::quat>(); // quaternion

  glm::mat4 get_model_matrix();
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Transform, translation, scale, rotation);
} // namespace ale

#endif // ALETHERENGINE_TRANSFORM_H
