//
// Created by Alether on 4/25/2024.
//

#ifndef ALETHERENGINE_OBJECT_H
#define ALETHERENGINE_OBJECT_H

#include<glm/glm.hpp>
#include<memory>
#include"transform.h"

using namespace glm;
using namespace std;

namespace ale {

class Model;

class Object {
public:
    Transform transform;
    shared_ptr<Model> model;
};
}

#endif //ALETHERENGINE_OBJECT_H
