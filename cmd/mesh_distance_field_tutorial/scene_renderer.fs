#version 430 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform vec3 viewPos;
uniform vec4 diffuseColor;

#include "cmd/mesh_distance_field_tutorial/sdf_atlas_3d_softshadow.fs"

float ShadowCalculation(vec3 fragPos, vec3 lightPos, vec3 normalDir)
{
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    vec3 isectPos = vec3(0.0);
    vec3 boxCenter = vec3(0.0);

    // + (normalDir * 0.05) -> prevents self intersect but cannot ignore self intersection fully because of possible self shadow
    // + (lightDir * 0.05) -> prevents bleeding by escaping the really close to boundaries sdfs
    return raymarch(fragPos + (normalDir * 0.05) + (lightDir * 0.05), lightDir, distance(lightPos, fragPos), isectPos, boxCenter);
}

void main()
{
    vec3 color = diffuseColor.rgb;
    vec3 normal = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 lighting = vec3(0.0);  // Accumulate lighting contributions


    vec3 lightColor = vec3(1.0);
    vec3 lightPos = vec3(5.0, 5.0, 3.0);
    vec3 lightAttenuation = vec3(1.0f, 0.09f, 0.032f);

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
    float attenuation = 1.0 / (lightAttenuation.x + lightAttenuation.y * distance +
        lightAttenuation.z * (distance * distance));
    float shadow = ShadowCalculation(fs_in.FragPos, lightPos, normal);

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    lighting += (ambient + (shadow) * (diffuse + specular)) * color;
    lighting *= color;

    float dither = fract(52.9829189 * fract(dot(gl_FragCoord.xy, vec2(0.06711056, 0.00583715))));
    lighting += (1.0 / 255.0) * dither - (0.5 / 255.0);;

    FragColor = vec4(lighting, 1.0);
//    FragColor = vec4(vec3(shadow), 1.0);
}