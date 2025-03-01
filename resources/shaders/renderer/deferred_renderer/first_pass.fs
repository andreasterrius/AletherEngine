#version 430 core
out vec4 FragColor;

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec4 gAlbedoSpec;
layout(location = 3) out ivec4 gEntityId;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform int entityId;
uniform vec3 diffuseColor;
uniform float specularColor;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D normalTexture;

void main()
{
    gPosition = fs_in.FragPos;
    gNormal = normalize(fs_in.Normal);
    gAlbedoSpec.rgb = texture(diffuseTexture, fs_in.TexCoords).rgb + diffuseColor;
    gAlbedoSpec.a = texture(specularTexture, fs_in.TexCoords).r + specularColor;
    gEntityId.r = entityId;
}