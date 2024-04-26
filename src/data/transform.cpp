//
// Created by Alether on 4/26/2024.
//

#include "transform.h"
using namespace glm;

mat4 ale::Transform::getModelMatrix() {
    //TODO: scale and rotation
    mat4 transform = mat4(1.0);
    transform = translate(transform, translation);
    return transform;
}
