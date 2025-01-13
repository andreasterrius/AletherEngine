//
// Created by Alether on 4/17/2024.
//

#ifndef ALETHERENGINE_BOUNDINGBOX_H
#define ALETHERENGINE_BOUNDINGBOX_H

#include "src/data/transform.h"
#include <glm/glm.hpp>

using namespace glm;

namespace ale {

class Transform;

class BoundingBox {
public:
  vec3 min, max, center;
  BoundingBox(vec3 min, vec3 max);

  BoundingBox apply_scale(Transform t) const;

  vec3 getCenter() const;

  vec3 getSize() const;

  bool isInside(vec3 p);
};
} // namespace ale

#endif // ALETHERENGINE_BOUNDINGBOX_H
