#include "sdf_generator_gpu_v2_shared.h"

float dot2(vec3 v) { return dot(v, v); }

float distance2(vec3 a, vec3 b) {
  float ff = distance(a, b);
  return ff * ff;
}

vec3 closest_point_on_segment(vec3 point, vec3 seg_start, vec3 seg_end) {
  vec3 segment = seg_end - seg_start;
  float t = dot(point - seg_start, segment) / dot(segment, segment);
#if GLSL
  t = clamp(t, 0.0f, 1.0f); // Clamp to segment bounds
#else
  t = glm::clamp(t, 0.0f, 1.0f); // Clamp to segment bounds
#endif
  return seg_start + t * segment;
}

vec3 closest_point_on_triangle(vec3 A, vec3 v0, vec3 v1, vec3 v2) {
  // Compute triangle normal
  vec3 edge1 = v1 - v0;
  vec3 edge2 = v2 - v0;
  vec3 normal = cross(edge1, edge2);

  // Handle degenerate triangles (zero-area)
  float normal_length = length(normal);
  if (normal_length < 1e-6f)
    return v0; // Return arbitrary vertex

  normal = normalize(normal);

  // Project point A onto the triangle's plane
  float dist = dot(A - v0, normal);
  vec3 A_proj = A - dist * normal;

  // Compute barycentric coordinates
  vec3 v0_to_proj = A_proj - v0;
  float d00 = dot(edge1, edge1);
  float d01 = dot(edge1, edge2);
  float d11 = dot(edge2, edge2);
  float d02 = dot(edge1, v0_to_proj);
  float d12 = dot(edge2, v0_to_proj);
  float denom = d00 * d11 - d01 * d01;

  if (abs(denom) < 1e-6f)
    return v0; // Degenerate triangle

  float v = (d11 * d02 - d01 * d12) / denom;
  float w = (d00 * d12 - d01 * d02) / denom;
  float u = 1.0f - v - w;

  // Check if projection is inside the triangle
  if (u >= 0.0f && v >= 0.0f && w >= 0.0f) {
    return A_proj;
  }

  // Project onto edges and find closest point
  vec3 candidates[3];
  candidates[0] = closest_point_on_segment(A, v0, v1);
  candidates[1] = closest_point_on_segment(A, v1, v2);
  candidates[2] = closest_point_on_segment(A, v2, v0);

  // Find closest candidate
  float min_dist_sq = 100000.0;
  vec3 closest = A_proj;
  for (int i = 0; i < 3; ++i) {
    float dist_sq = distance(A, candidates[i]);
    if (dist_sq < min_dist_sq) {
      min_dist_sq = dist_sq;
      closest = candidates[i];
    }
  }

  return closest;
}

void generate_sdf(ivec3 texel_coord, int vertices_size, int indices_size,
                  vec3 outer_bb_min, vec3 outer_bb_max, ivec3 image_size
#if !(GLSL)
                  ,
                  vector<Vertex> &vertices, vector<unsigned int> &indices,
                  vector<vector<vector<vec4>>> &imgOutput, vec3 &chosen_normal
#endif
) {

  vec3 cube_size = vec3((outer_bb_max.x - outer_bb_min.x) / image_size.x,
                        (outer_bb_max.y - outer_bb_min.y) / image_size.y,
                        (outer_bb_max.z - outer_bb_min.z) / image_size.z);
  vec3 cube_center_pos =
      (outer_bb_min + cube_size / vec3(2)) + cube_size * vec3(texel_coord);

  // float shortest_distance = 1000000.0;
  float shortest_check_distance = 1000000.0;
  float shortest_distance = 1000000.0;
  vec3 shortest_point = vec3(0.0);
  vec3 shortest_normal = vec3(0.0);
  for (int i = 0; i < indices_size; i += 3) {
    Vertex a = vertices[indices[i]];
    Vertex b = vertices[indices[i + 1]];
    Vertex c = vertices[indices[i + 2]];

    vec3 closest_point = closest_point_on_triangle(
        cube_center_pos, vec3(a.position), vec3(b.position), vec3(c.position));
    vec3 ab = vec3(b.position) - vec3(a.position);
    vec3 ac = vec3(c.position) - vec3(a.position);
    vec3 normal = normalize(cross(ab, ac));
    float check_dist =
        distance(closest_point + normal * vec3(0.001), cube_center_pos);
    if (check_dist < shortest_check_distance) {
      shortest_check_distance = check_dist;
      shortest_distance = distance(closest_point, cube_center_pos);
      shortest_point = closest_point;
      shortest_normal = normal;
    }
  }
  float t = dot(shortest_normal, normalize(cube_center_pos - shortest_point));
  if (t < 0) {
    shortest_distance = -abs(shortest_distance);
  }

#if !(GLSL)
  chosen_normal = shortest_normal;
  imgOutput[texel_coord.x][texel_coord.y][texel_coord.z] = vec4(
      shortest_distance, shortest_point.x, shortest_point.y, shortest_point.z);
#else
  imageStore(imgOutput, texel_coord, vec4(shortest_distance, 0.0, 0.0, 0.0));
#endif
}