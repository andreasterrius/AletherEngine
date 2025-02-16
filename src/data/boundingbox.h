//
// Created by Alether on 4/17/2024.
//

#ifndef ALETHERENGINE_BOUNDINGBOX_H
#define ALETHERENGINE_BOUNDINGBOX_H

#include "src/data/transform.h"
#include <glm/glm.hpp>

namespace ale {

class Transform;

class BoundingBox {
public:
  glm::vec3 min, max, center;
  BoundingBox(glm::vec3 min, glm::vec3 max);

  BoundingBox apply_scale(Transform t) const;

  glm::vec3 getCenter() const;

  glm::vec3 getSize() const;

  bool isInside(glm::vec3 p);
};
} // namespace ale

#endif // ALETHERENGINE_BOUNDINGBOX_H
