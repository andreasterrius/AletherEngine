//
// Created by Alether on 4/26/2024.
//

#include "transform.h"
using namespace glm;

mat4 ale::Transform::getModelMatrix() {
    mat4 translation = glm::translate(mat4(1.0f), this->translation);
    mat4 rotation = glm::mat4_cast(this->rotation);
    mat4 scale = glm::scale(mat4(1.0f), this->scale);

    return rotation * translation  * scale;
}