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

    static float distanceFromBox(vec3 p, vec3 min, vec3 max);

    static vec3 getFaceNormal(const vec3 v0, const vec3 v1, const vec3 v2);

    static vec3 getFaceCenter(const vec3 v0, const vec3 v1, const vec3 v2);

    static bool rayTriangleIntersect(vec3 orig, vec3 dir, vec3 v0, vec3 v1, vec3 v2, float &t);
};


#endif //ALETHERENGINE_UTIL_H
