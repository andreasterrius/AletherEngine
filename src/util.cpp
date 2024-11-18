//
// Created by Alether on 4/16/2024.
//

#include "util.h"

#include "glm/gtc/epsilon.hpp"

std::optional<unsigned int> Util::loadTexture(const char *path) {
  unsigned int textureID;
  glGenTextures(1, &textureID);

  int width, height, nrComponents;
  unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
  if (data) {
    GLenum format;
    if (nrComponents == 1)
      format = GL_RED;
    else if (nrComponents == 3)
      format = GL_RGB;
    else if (nrComponents == 4)
      format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(
        GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
        format == GL_RGBA
            ? GL_CLAMP_TO_EDGE
            : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent
                          // semi-transparent borders. Due to interpolation it
                          // takes texels from next repeat
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                    format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  } else {
    stbi_image_free(data);
    return std::nullopt;
  }

  return textureID;
}

float Util::dot2(vec2 v) { return dot(v, v); }

float Util::dot2(vec3 v) { return dot(v, v); }

float Util::distanceFromBox(vec3 p, vec3 min, vec3 max) {
  // Calculate the center and half-size of the box
  vec3 center = (min + max) * 0.5f;
  vec3 halfSize = (max - min) * 0.5f;

  // Calculate the distance from the point to the center of the box
  vec3 d = abs(p - center) - halfSize;

  // Calculate the distance to the box
  return length(glm::max(d, 0.0f)) +
         glm::min(glm::max(d.x, glm::max(d.y, d.z)), 0.0f);
}

vec3 Util::getFaceNormal(const vec3 v0, const vec3 v1, const vec3 v2) {
  vec3 edge1 = v1 - v0;
  vec3 edge2 = v2 - v0;
  vec3 normal = cross(edge1, edge2);
  return normalize(normal);
}

vec3 Util::getFaceCenter(const vec3 v0, const vec3 v1, const vec3 v2) {
  vec3 centroid;
  centroid.x = (v0.x + v1.x + v2.x) / 3.0;
  centroid.y = (v0.y + v1.y + v2.y) / 3.0;
  centroid.z = (v0.z + v1.z + v2.z) / 3.0;
  return centroid;
}

float Util::udTriangle(vec3 p, vec3 a, vec3 b, vec3 c) {
  vec3 ba = b - a;
  vec3 pa = p - a;
  vec3 cb = c - b;
  vec3 pb = p - b;
  vec3 ac = a - c;
  vec3 pc = p - c;
  vec3 nor = cross(ba, ac);

  return sqrt(
      (sign(dot(cross(ba, nor), pa)) + sign(dot(cross(cb, nor), pb)) +
           sign(dot(cross(ac, nor), pc)) <
       2.0)
          ? min(min(dot2(ba * clamp(dot(ba, pa) / dot2(ba), 0.0f, 1.0f) - pa),
                    dot2(cb * clamp(dot(cb, pb) / dot2(cb), 0.0f, 1.0f) - pb)),
                dot2(ac * clamp(dot(ac, pc) / dot2(ac), 0.0f, 1.0f) - pc))
          : dot(nor, pa) * dot(nor, pa) / dot2(nor));
}

bool Util::rayTriangleIntersect(vec3 orig, vec3 dir, vec3 v0, vec3 v1, vec3 v2,
                                float &t) {
  const float EPSILON = 1e-6f;
  glm::vec3 edge1 = v1 - v0;
  glm::vec3 edge2 = v2 - v0;
  glm::vec3 h = glm::cross(dir, edge2);
  float a = glm::dot(edge1, h);
  if (glm::epsilonEqual(a, 0.0f, EPSILON)) {
    return false; // Ray is parallel to the triangle
  }
  float f = 1.0f / a;
  glm::vec3 s = orig - v0;
  float u = f * glm::dot(s, h);
  if (u < 0.0f || u > 1.0f) {
    return false;
  }
  glm::vec3 q = glm::cross(s, edge1);
  float v = f * glm::dot(dir, q);
  if (v < 0.0f || u + v > 1.0f) {
    return false;
  }
  // At this stage, we can compute t to find out where the intersection point is
  // on the line
  t = f * glm::dot(edge2, q);
  if (t > EPSILON) { // Ray intersection
    return true;
  } else { // Line intersection but not a ray intersection
    return false;
  }
}
