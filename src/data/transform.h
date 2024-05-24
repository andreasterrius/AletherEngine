//
// Created by Alether on 4/26/2024.
//

#ifndef ALETHERENGINE_TRANSFORM_H
#define ALETHERENGINE_TRANSFORM_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

namespace ale {
class Transform {
public:
    vec3 translation = vec3(0.0f);
    vec3 scale = vec3(1.0f);
    vec4 rotation = vec4(0.0f); //quaternion

    mat4 getModelMatrix();
};
}


#endif //ALETHERENGINE_TRANSFORM_H
