#version 330 core

in vec2 TexCoord;
out vec4 color;

uniform sampler2D texture1;
uniform bool discard_alpha;

void main()
{
    vec4 tex_color = texture(texture1, TexCoord);
    if(discard_alpha && tex_color.a == 0.0) {
        discard;
    }
    color = tex_color;
}