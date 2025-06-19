//
// Created by Alether on 4/13/2025.
//
module;

#include <glm/glm.hpp>

export module data:color;

export namespace ale {
constexpr glm::vec3 WHITE = glm::vec3(1.0, 1.0, 1.0);
constexpr glm::vec3 RED = glm::vec3(1.0, 0.0, 0.0);
constexpr glm::vec3 GREEN = glm::vec3(0.0, 1.0, 0.0);
constexpr glm::vec3 BLUE = glm::vec3(0.0, 0.0, 1.0);
constexpr glm::vec3 YELLOW = glm::vec3(1.0, 1.0, 0.0);
constexpr glm::vec3 BLUE_SKY =
    glm::vec3(135.0 / 255.0, 206 / 255.0, 235 / 255.0);
} // namespace ale
