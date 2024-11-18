//
// Created by Alether on 4/25/2024.
//

#ifndef ALETHERENGINE_OBJECT_H
#define ALETHERENGINE_OBJECT_H

#include "transform.h"
#include <glm/glm.hpp>
#include <memory>

using namespace glm;
using namespace std;

namespace ale {

class Model;

class Object {
public:
  Transform transform;
  shared_ptr<Model> model;
  bool shouldRender = true;

  vec4 color;
};
} // namespace ale

#endif // ALETHERENGINE_OBJECT_H
