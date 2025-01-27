#version 430 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout (r32f, binding = 0) uniform image3D imgOutput;


void main() {
    ivec3 texel_coord = ivec3(gl_GlobalInvocationID.xyz);
    imageStore(imgOutput, texel_coord, vec4(0.5, 0.0, 0.0, 0.0));
}