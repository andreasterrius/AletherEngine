#version 430 core

#define GLSL 1

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout (r32f, binding = 0) uniform image3D imgOutput;

struct Vertex {
    vec4 position;
    vec4 normal;
    vec2 tex_coords;
    vec4 tangent;
    vec4 bitangent;
    ivec4 bone_ids;
    vec4 weights;
};
layout (std430, binding = 2) buffer VertexBuffer {
    Vertex vertices[];
};
layout (std430, binding = 3) buffer IndexBuffer {
    uint indices[];
};
layout (std140, binding = 4) uniform BoundingBox {
    ivec4 buffer_size;
    vec4 inner_bb_min;
    vec4 inner_bb_max;
    vec4 outer_bb_min;
    vec4 outer_bb_max;
};

#include "src/graphics/sdf/sdf_generator_gpu_v2_shared.cpp"

void main() {
    int vertices_size = buffer_size.x;
    int indices_size = buffer_size.y;
    ivec3 image_size = imageSize(imgOutput);
    generate_sdf(ivec3(gl_GlobalInvocationID.xyz), vertices_size, indices_size,
        vec3(outer_bb_min), vec3(outer_bb_max), image_size);
}