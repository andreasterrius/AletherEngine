#version 430 core
out vec4 FragColor;

in vec2 TexCoords;

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

uniform vec3 ambientColor;
uniform float ambientIntensity;

uniform vec3 viewPos;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D gEntityId;
uniform sampler2D gDepth;

#include "resources/shaders/sdf/sdf_atlas_partial.fs"

float ShadowCalculation(vec3 fragPos, vec3 lightPos, vec3 normalDir)
{
    vec3 lightDir = normalize(lightPos - fragPos);
    vec3 isectPos = vec3(0.0);
    vec3 boxCenter = vec3(0.0);

    // + (normalDir * 0.05) -> prevents self intersect but cannot ignore self intersection fully because of possible self shadow
    // + (lightDir * 0.05) -> prevents bleeding by escaping the really close to boundaries sdfs
    return raymarch(fragPos + (normalDir * 0.05) + (lightDir * 0.05), lightDir, distance(lightPos, fragPos), isectPos, boxCenter);
//    return raymarch(fragPos + lightDir * 0.01, lightDir, distance(lightPos, fragPos), isectPos, boxCenter);
}

void main()
{
    vec3 position = texture(gPosition, TexCoords).rgb;
    vec3 color = texture(gAlbedoSpec, TexCoords).rgb;
    vec3 normal = texture(gNormal, TexCoords).rgb;
    vec3 viewDir = normalize(viewPos - position);
    float depth = texture(gDepth, TexCoords).r;
    vec3 lighting = vec3(0.0);  // Accumulate lighting contributions

    if (depth == 1.0) {
        discard;
    }

    for(int i = 0; i < numLights; ++i) {
        vec3 lightColor = lights[i].color;
        vec3 lightPos = lights[i].position;

        // diffuse
        vec3 lightDir = normalize(lightPos - position);
        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;

        // specular
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = 0.0;
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
        vec3 specular = spec * lightColor;

        // calculate shadow
        float distance = length(lightPos - position);
        float attenuation = 1.0 / (lights[i].attenuation.x + lights[i].attenuation.y * distance +
        lights[i].attenuation.z * (distance * distance));
        float shadow = ShadowCalculation(position, lightPos, normal);

        diffuse *= attenuation;
        specular *= attenuation;

        lighting += shadow * (diffuse + specular);
    }

    vec3 ambient = ambientIntensity * ambientColor;
    lighting = ambient + (lighting * color);

    float dither = fract(52.9829189 * fract(dot(gl_FragCoord.xy, vec2(0.06711056, 0.00583715))));
    lighting += (1.0 / 255.0) * dither - (0.5 / 255.0);;

    FragColor = vec4(vec3(lighting), 1.0);
}