#version 430 core
out vec4 FragColor;

layout(location = 1) out vec3 gPosition;
layout(location = 2) out vec3 gNormal;
layout(location = 3) out vec4 gAlbedoSpec;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

struct Light {
    vec3 position;
    vec3 color;

    // used for soft shadows, not for color calculation (yet)
    float radius;

    // for point lights
    vec3 attenuation;
};
uniform int numLights;
uniform Light lights[20];

uniform vec3 viewPos;
uniform vec4 diffuseColor;

uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;
//uniform sampler2D specularTexture;
//uniform sampler2D roughnessTexture;
//uniform sampler2D metalnessTexture;
//uniform sampler2D aoTexture;

#include "resources/shaders/sdf/sdf_atlas_partial.fs"

float ShadowCalculation(vec3 fragPos, vec3 lightPos, vec3 normalDir)
{
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    vec3 isectPos = vec3(0.0);
    vec3 boxCenter = vec3(0.0);

    // + (normalDir * 0.05) -> prevents self intersect but cannot ignore self intersection fully because of possible self shadow
    // + (lightDir * 0.05) -> prevents bleeding by escaping the really close to boundaries sdfs
    bool occluded = raymarch(fragPos + (normalDir * 0.05) + (lightDir * 0.05), lightDir, distance(lightPos, fragPos), isectPos, boxCenter);

    if (occluded) {
        return 1.0;
    }
    return 0.0;
}

void main()
{
    vec3 color = diffuseColor.rgb;
    vec3 normal = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 lighting = vec3(0.0);  // Accumulate lighting contributions

    vec3 diffuseFinal = vec3(0.0);
    vec3 specularFinal = vec3(0.0);
    for(int i = 0; i < numLights; ++i) {
        vec3 lightColor = lights[i].color;
        vec3 lightPos = lights[i].position;

        // ambient
        vec3 ambient = 0.3 * lightColor;

        // diffuse
        vec3 lightDir = normalize(lightPos - fs_in.FragPos);
        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;

        // specular
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = 0.0;
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
        vec3 specular = spec * lightColor;

        // calculate shadow
        float distance = length(lightPos - fs_in.FragPos);
        float attenuation = 1.0 / (lights[i].attenuation.x + lights[i].attenuation.y * distance +
            lights[i].attenuation.z * (distance * distance));
        float shadow = ShadowCalculation(fs_in.FragPos, lightPos, normal);

        ambient *= attenuation;
        diffuse *= attenuation;
        specular *= attenuation;

        lighting += (ambient + (1.0 - shadow) * (diffuse + specular)) * color;
        specularFinal += specular;
        diffuseFinal += diffuse;
    }

    lighting *= color;

    FragColor = vec4(lighting, 1.0);
    gPosition = fs_in.FragPos;
    gNormal = normalize(fs_in.Normal);
    gAlbedoSpec = vec4(diffuseFinal, specularFinal);
}