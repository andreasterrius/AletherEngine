
#ifndef SDF_GENERATOR_GPU_V2_SHARED_H
#define SDF_GENERATOR_GPU_V2_SHARED_H

#define GLSL 0

#include "../data/model.h"
#include "glm/glm.hpp"
#include <vector>

using namespace glm;

float dot2(vec3 v);

float distance2(vec3 a, vec3 b);

// Helper function to project a point onto a line segment
vec3 closest_point_on_segment(vec3 point, vec3 seg_start, vec3 seg_end);

vec3 closest_point_on_triangle(vec3 A, vec3 v0, vec3 v1, vec3 v2);

void generate_sdf(ivec3 texel_coord, int vertices_size, int indices_size,
                  vec3 outer_bb_min, vec3 outer_bb_max, ivec3 image_size,
                  vector<Vertex> &vertices, vector<unsigned int> &indices,
                  vector<vector<vector<vec4>>> &imgOutput,
                  vec3 &closest_normal);

#endif // SDF_GENERATOR_GPU_V2_SHARED_H
