
#ifndef SDF_GENERATOR_GPU_V2_SHARED_H
#define SDF_GENERATOR_GPU_V2_SHARED_H

#define GLSL 0

#include <vector>
#include "glm/glm.hpp"

import graphics;

float dot2(glm::vec3 v);

float distance2(glm::vec3 a, glm::vec3 b);

// Helper function to project a point onto a line segment
glm::vec3 closest_point_on_segment(glm::vec3 point, glm::vec3 seg_start,
                                   glm::vec3 seg_end);

glm::vec3 closest_point_on_triangle(glm::vec3 A, glm::vec3 v0, glm::vec3 v1,
                                    glm::vec3 v2);

void generate_sdf(glm::ivec3 texel_coord, int vertices_size, int indices_size,
                  glm::vec3 outer_bb_min, glm::vec3 outer_bb_max,
                  glm::ivec3 image_size,
                  std::vector<ale::graphics::Vertex> &vertices,
                  std::vector<unsigned int> &indices,
                  std::vector<std::vector<std::vector<glm::vec4>>> &imgOutput,
                  glm::vec3 &closest_normal);

#endif // SDF_GENERATOR_GPU_V2_SHARED_H
