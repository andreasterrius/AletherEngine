//
// Created by Alether on 4/16/2024.
//

#ifndef ALETHERENGINE_UTIL_H
#define ALETHERENGINE_UTIL_H

#include <glm/glm.hpp>
#include <optional>

class Util {
public:
  [[deprecated("use Texture(string) ctor instead")]]
  static std::optional<unsigned int> loadTexture(char const *path);

  static float udTriangle(glm::vec3 p, glm::vec3 a, glm::vec3 b, glm::vec3 c);

  static float dot2(glm::vec2 v);

  static float dot2(glm::vec3 v);

  static float distanceFromBox(glm::vec3 p, glm::vec3 min, glm::vec3 max);

  static glm::vec3 getFaceNormal(const glm::vec3 v0, const glm::vec3 v1,
                                 const glm::vec3 v2);

  static glm::vec3 getFaceCenter(const glm::vec3 v0, const glm::vec3 v1,
                                 const glm::vec3 v2);

  static bool rayTriangleIntersect(glm::vec3 orig, glm::vec3 dir, glm::vec3 v0,
                                   glm::vec3 v1, glm::vec3 v2, float &t);
};

#endif // ALETHERENGINE_UTIL_H
