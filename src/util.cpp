//
// Created by Alether on 4/16/2024.
//

#include "util.h"

std::optional<unsigned int> Util::loadTexture(const char *path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        stbi_image_free(data);
        return std::nullopt;
    }

    return textureID;
}

float Util::dot2(vec2 v) { return dot(v,v); }

float Util::dot2(vec3 v) { return dot(v,v); }

float Util::distanceFromBox(vec3 p, vec3 min, vec3 max) {
    // Calculate the center and half-size of the box
    vec3 center = (min + max) * 0.5f;
    vec3 halfSize = (max - min) * 0.5f;

    // Calculate the distance from the point to the center of the box
    vec3 d = abs(p - center) - halfSize;

    // Calculate the distance to the box
    return length(glm::max(d, 0.0f)) + glm::min(glm::max(d.x, glm::max(d.y, d.z)), 0.0f);
}

float Util::udTriangle( vec3 p, vec3 a, vec3 b, vec3 c )
{
    vec3 ba = b - a; vec3 pa = p - a;
    vec3 cb = c - b; vec3 pb = p - b;
    vec3 ac = a - c; vec3 pc = p - c;
    vec3 nor = cross( ba, ac );

    return sqrt(
      (sign(dot(cross(ba,nor),pa)) +
       sign(dot(cross(cb,nor),pb)) +
       sign(dot(cross(ac,nor),pc))<2.0)
       ?
       min( min(
       dot2(ba*clamp(dot(ba,pa)/dot2(ba),0.0f,1.0f)-pa),
       dot2(cb*clamp(dot(cb,pb)/dot2(cb),0.0f,1.0f)-pb) ),
       dot2(ac*clamp(dot(ac,pc)/dot2(ac),0.0f,1.0f)-pc) )
       :
       dot(nor,pa)*dot(nor,pa)/dot2(nor) );
}
