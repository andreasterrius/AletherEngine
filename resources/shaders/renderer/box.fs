#version 330 core

in vec3 outColor;

void main()
{
    gl_FragColor = vec4(outColor, 0.0);
}
