#version 330 core

out vec4 out_color;

uniform vec2	iResolution;
uniform float	iTime;
uniform mat4    invViewProj;
uniform vec3    cameraPos;

float distance_from_sphere(in vec3 p, in vec3 c, float r)
{
    return length(p - c) - r;
}

vec3 raymarch(vec3 ro, vec3 rd){
    float total_distance_traveled = 0.0;
    const int NUMBER_OF_STEPS = 32;
    const float MINIMUM_HIT_DISTANCE = 0.001;
    const float MAXIMUM_TRACE_DISTANCE = 1000.0;

    for (int i = 0; i < NUMBER_OF_STEPS; ++i)
    {
        vec3 current_position = ro + total_distance_traveled * rd;

        float distance_to_closest = distance_from_sphere(current_position, vec3(0.0), 1.0);

        if (distance_to_closest < MINIMUM_HIT_DISTANCE)
        {
            return vec3(1.0, 0.0, 0.0);
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