//
// Created by Alether on 4/26/2024.
//

module;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nlohmann/json.hpp>
#include "src/data/serde/glm.h"

export module transform;
using namespace glm;

export namespace ale::data {

class Transform {
public:
  glm::vec3 translation = glm::vec3(0.0f);
  glm::vec3 scale = glm::vec3(1.0f);
  glm::quat rotation = glm::identity<glm::quat>(); // quaternion

  glm::mat4 get_model_matrix() {
    mat4 translation = glm::translate(mat4(1.0f), this->translation);
    mat4 rotation = glm::mat4_cast(this->rotation);
    mat4 scale = glm::scale(mat4(1.0f), this->scale);

    return translation * rotation * scale;
  }
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Transform, translation, scale, rotation);
} // namespace ale::data
