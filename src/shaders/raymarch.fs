#version 330 core

out vec4 out_color;

uniform vec2	iResolution;
uniform float	iTime;
uniform mat4    invViewProj;
uniform vec3    cameraPos;

uniform vec3 bbMin;
uniform vec3 bbSize;
uniform vec3 textureSize;
uniform sampler3D texture3D;

vec3 ConvertWorldToTexture(vec3 worldPos, vec3 boxMin, vec3 boxSize)
{
    vec3 texturePos = (worldPos - boxMin) / boxSize;
    return texturePos;
}

float distance_from_texture3D(vec3 p)
{
    vec3 uvwCoord = ConvertWorldToTexture(p, bbMin, bbSize);
    return texture(texture3D, uvwCoord).r;
}

float distance_from_sphere(in vec3 p, in vec3 c, float r)
{
    return length(p - c) - r;
}

float distance_from_box(vec3 p, vec3 size) {
    vec3 q = abs(p) - size/2.0f;
    return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

vec3 raymarch(vec3 ro, vec3 rd){
    float total_distance_traveled = 0.0;
    const int NUMBER_OF_STEPS = 32;
    const float MINIMUM_HIT_DISTANCE = 0.001;
    const float MAXIMUM_TRACE_DISTANCE = 1000.0;

    for (int i = 0; i < NUMBER_OF_STEPS; ++i)
    {
        vec3 current_position = ro + total_distance_traveled * rd;

        //float distance_to_closest = distance_from_sphere(current_position, vec3(0.0), 1.0);
        //float distance_to_closest = distance_from_texture3D(current_position);
        float distance_to_closest = distance_from_box(current_position, bbSize);
        if (distance_to_closest < MINIMUM_HIT_DISTANCE)
        {
            distance_to_closest = distance_from_texture3D(current_position);
            if (distance_to_closest < MINIMUM_HIT_DISTANCE)
            {
                return vec3(1.0, 0.0, 0.0);
            }
        }

        if (total_distance_traveled > MAXIMUM_TRACE_DISTANCE)
        {
            break;
        }
        total_distance_traveled += distance_to_closest;
    }
    return vec3(0.0);
}

void main()
{
    // Current point we're hitting
    vec2 uv = gl_FragCoord.xy / iResolution.xy * 2.0 - 1.0;

    vec4 rayStartWorld = vec4(cameraPos, 1.0);
    vec4 rayEndWorld = invViewProj * vec4(uv, 0.0, 1.0);
    rayEndWorld /= rayEndWorld.w;

    vec3 rayDir = vec3(normalize(rayEndWorld - rayStartWorld));

    vec3 color = raymarch(vec3(rayStartWorld), rayDir);
    out_color = vec4(color, 1.0);
}