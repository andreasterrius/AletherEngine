#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 7) in vec3 bbMin;
layout (location = 8) in vec3 bbMax;
layout (location = 9) in vec3 color;

out vec3 outColor;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    vec3 tPos = (vec3(aPos) + 1.0) / 2; // transform to 0-1 cube
    vec3 nPos = mix(bbMin, bbMax, tPos);
    gl_Position = projection * view * vec4(nPos, 1.0);
    outColor = color;
}