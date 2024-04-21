//
// Created by Alether on 4/17/2024.
//

#ifndef ALETHERENGINE_BOUNDINGBOX_H
#define ALETHERENGINE_BOUNDINGBOX_H

#include<glm/glm.hpp>

using namespace glm;

namespace ale {
class BoundingBox {
public:
    vec3 min, max;
    BoundingBox(vec3 min, vec3 max);
};
}


#endif //ALETHERENGINE_BOUNDINGBOX_H
