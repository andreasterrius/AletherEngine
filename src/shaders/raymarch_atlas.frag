#version 430 core

out vec4 out_color;

uniform vec2 iResolution;
uniform mat4 invViewProj;
uniform vec3 cameraPos;

#include "src/shaders/partial/sdf_atlas.fs"

vec3 lambertBRDF(vec3 normal, vec3 lightDir, vec3 albedo) {
    float NdotL = max(dot(normal, lightDir), 0.0);
    return albedo * NdotL / 3.14;
}

void main()
{
    const vec3 NO_HIT_COLOR = vec3(0.52, 0.8, 0.92);
    const vec3 SDF_COLOR =  vec3(0.89, 0.89, 0.56);

    // Current point we're hitting
    vec2 uv = gl_FragCoord.xy / iResolution.xy * 2.0 - 1.0;

    vec4 rayStartWorld = vec4(cameraPos, 1.0);
    vec4 rayEndWorld = invViewProj * vec4(uv, 0.0, 1.0);
    rayEndWorld /= rayEndWorld.w;

    vec3 rayDir = vec3(normalize(rayEndWorld - rayStartWorld));

    vec3 isectPos = vec3(0.0);
    vec3 objCenter = vec3(0.0);

    // make a light because why not
    const vec3 lightPos = vec3(3.0, 5.0, 5.0);
    const vec3 lightColor = vec3(3.0, 3.0, 3.0);

    bool hit = raymarch(vec3(rayStartWorld), rayDir, 10000.0, isectPos, objCenter);
    vec3 color = vec3(NO_HIT_COLOR);
    if(hit) {
        vec3 normal = normalize(isectPos - objCenter);
        color = lambertBRDF(normal, normalize(lightPos - isectPos), SDF_COLOR);
    }
    // color = pow(color, vec3(1.0 / 2.2));

    out_color = vec4(color, 1.0);
}
