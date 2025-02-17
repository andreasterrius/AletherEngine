//
// Created by Alether on 4/17/2024.
//

#ifndef ALETHERENGINE_RAY_H
#define ALETHERENGINE_RAY_H

#include <glm/glm.hpp>
#include <optional>
#include <string>

namespace ale {

class BoundingBox;
class Transform;

class Ray {
public:
  glm::vec3 origin;
  glm::vec3 dir;
  glm::vec3 invDir;

  Ray(glm::vec3 origin, glm::vec3 dir) : origin(origin), dir(normalize(dir)) {
    this->invDir = glm::vec3(1.0f / dir.x, 1.0f / dir.y, 1.0f / dir.z);
  }

  std::optional<float> intersect(const BoundingBox &box, float limitTMin = 0,
                                 float limitTMax = INFINITY);

  glm::vec3 resolve(float t);

  Ray apply_transform_inversed(Transform t);

  std::string to_string() {
    return "o:(" + std::to_string(origin.x) + "." + std::to_string(origin.y) +
           "," + std::to_string(origin.z) + ")" + " | d:(" +
           std::to_string(dir.x) + "." + std::to_string(dir.y) + "," +
           std::to_string(dir.z) + ")";
  }
};

} // namespace ale

#endif // ALETHERENGINE_RAY_H
