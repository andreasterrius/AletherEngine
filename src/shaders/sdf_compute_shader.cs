#version 430 core

layout (local_size_x = 8) in;
layout (rgba32f, binding = 0) uniform image2D imgOutput;

shared vec3 sharedColors[8];

void main(){
    ivec2 texelCoord = ivec2(gl_WorkGroupID.xy);

    vec3 value = vec3(0.0, 0.0, 0.0);
    value.x = gl_LocalInvocationID.x * 0.1;
    value.y = gl_LocalInvocationID.x * 0.1;
    value.z = gl_LocalInvocationID.x * 0.1;
    sharedColors[gl_LocalInvocationIndex] = value;

    barrier();

    for (uint s=gl_WorkGroupSize.x/2; s>0; s>>=1) {
        if (gl_LocalInvocationIndex < s) {
            sharedColors[gl_LocalInvocationIndex] += sharedColors[gl_LocalInvocationIndex+s];
        }
        barrier();
    }

    if(gl_LocalInvocationIndex == 0){
//          float color = sharedColors[0].x/gl_WorkGroupSize.x;
        float color = sharedColors[0].x * 0.1;
        imageStore(imgOutput, texelCoord, vec4(vec3(color), 1.0));
    }
}
