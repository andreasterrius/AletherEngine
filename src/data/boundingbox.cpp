//
// Created by Alether on 4/17/2024.
//

#include "boundingbox.h"
#include "transform.h"

using namespace glm;
using namespace std;

ale::BoundingBox::BoundingBox(vec3 min, vec3 max) : min(min), max(max) {
  this->center = vec3(min.x + (max.x - min.x) / 2, min.y + (max.y - min.y) / 2,
                      min.z + (max.z - min.z) / 2);
}

ale::BoundingBox ale::BoundingBox::apply_scale(ale::Transform t) const {
  vec3 newSize = t.get_model_matrix() * vec4(this->getSize(), 1.0);
  vec3 newMin = this->getCenter() - newSize / 2.0f;
  vec3 newMax = this->getCenter() + newSize / 2.0f;
  return BoundingBox(newMin, newMax);
}

vec3 ale::BoundingBox::getCenter() const { return this->center; }

vec3 ale::BoundingBox::getSize() const { return this->max - this->min; }

bool ale::BoundingBox::isInside(vec3 p) {
  return (min[0] <= p[0] && p[0] <= max[0] && min[1] <= p[1] &&
          p[1] <= max[1] && min[2] <= p[2] && p[2] <= max[2]);
}
