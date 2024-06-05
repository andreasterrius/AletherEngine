//
// Created by Alether on 4/16/2024.
//

#ifndef ALETHERENGINE_UTIL_H
#define ALETHERENGINE_UTIL_H

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <stb_image.h>
#include <optional>

using namespace glm;

class Util {
public:
    static std::optional<unsigned int> loadTexture(char const * path);

    static float udTriangle( vec3 p, vec3 a, vec3 b, vec3 c );

    static float dot2( vec2 v );

    static float dot2( vec3 v );
};


#endif //ALETHERENGINE_UTIL_H
