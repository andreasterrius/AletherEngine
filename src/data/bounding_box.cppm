//
// Created by Alether on 4/17/2024.
//
module;

#include <glm/glm.hpp>

export module data:bounding_box;
import :transform;

using namespace glm;

export namespace ale::data {

class BoundingBox {
public:
  vec3 min, max, center;
  BoundingBox(vec3 min, vec3 max) : min(min), max(max) {
    this->center =
        vec3(min.x + (max.x - min.x) / 2, min.y + (max.y - min.y) / 2,
             min.z + (max.z - min.z) / 2);
  }

  BoundingBox apply_scale(Transform t) const {
    vec3 newSize = t.get_model_matrix() * vec4(this->getSize(), 1.0);
    vec3 newMin = this->getCenter() - newSize / 2.0f;
    vec3 newMax = this->getCenter() + newSize / 2.0f;
    return BoundingBox(newMin, newMax);
  }

  vec3 getCenter() const { return this->center; }

  vec3 getSize() const { return this->max - this->min; }

  bool isInside(vec3 p) {
    return (min[0] <= p[0] && p[0] <= max[0] && min[1] <= p[1] &&
            p[1] <= max[1] && min[2] <= p[2] && p[2] <= max[2]);
  }
};
} // namespace ale::data
