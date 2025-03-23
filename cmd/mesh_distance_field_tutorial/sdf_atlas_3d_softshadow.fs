
struct MonkeyMdfDetail {
    mat4 modelMat;
    mat4 invModelMat;
    vec3 innerBBMin;
    vec3 innerBBMax;
    vec3 outerBBMin;
    vec3 outerBBMax;
    vec3 resolution;
};

uniform MonkeyMdfDetail mdf;
uniform sampler3D monkey_mdf;

vec3 convert_world_to_texture(vec3 worldPos, vec3 boxMin, vec3 boxSize)
{
    return (worldPos - boxMin) / (boxSize);
}

float distance_from_texture3D(vec3 p, vec3 outerBBMin, vec3 outerBBMax)
{
    vec3 uvwCoord = convert_world_to_texture(p, outerBBMin, outerBBMax-outerBBMin);
    return texture(monkey_mdf, uvwCoord).r;
}

float distance_from_box_minmax(vec3 p, vec3 bbMin, vec3 bbMax) {
    vec3 d = max(p - bbMax, bbMin - p);

    float outsideDist = length(max(d, 0.0));
    float insideDist = min(max(d.x, max(d.y, d.z)), 0.0);

    return outsideDist + insideDist;
}

float get_scale_factor(mat4 invTransform, vec3 dir) {
    return (length(mat3(invTransform) * dir) + 0.00001);
}

float raymarch(vec3 rayWo, vec3 rayWd, float maxTraceDist, out vec3 isectPos, out vec3 objectCenter) {

    const int NUMBER_OF_STEPS = 128;
    const float MINIMUM_HIT_DISTANCE = 0.001;
    const float MAXIMUM_TRACE_DISTANCE = maxTraceDist;
    const float MAX_STEP_DIST = max(2.0, maxTraceDist/NUMBER_OF_STEPS/2.0);
    const float MIN_STEP_DIST = 0.02;
    const vec3 NO_HIT_COLOR = vec3(0.52, 0.8, 0.92);
    const vec3 SDF_COLOR =  vec3(0.89, 0.89, 0.56);
    const vec3 oriRayWo = rayWo;

    // for soft shadows
    float shadow = 1.0;
    const float k = 64;
    float t = 0.0;

    for (int i = 0; i < NUMBER_OF_STEPS; ++i)
    {
        vec3 rayLo = vec3(mdf.invModelMat * vec4(rayWo, 1.0));
        vec3 rayLd = vec3(normalize(mdf.invModelMat * vec4(rayWd, 0.0)));
        float scaleFactor = get_scale_factor(mdf.invModelMat, rayWd);

        float dist = distance_from_box_minmax(rayLo, mdf.innerBBMin, mdf.innerBBMax);
        float outerDist = distance_from_box_minmax(rayLo, mdf.outerBBMin, mdf.outerBBMax);
        if(outerDist < 0.0) {
            dist = distance_from_texture3D(rayLo, mdf.outerBBMin, mdf.outerBBMax);
            shadow = min(shadow, dist*k/(t+0.0001));
        }

        t += clamp(dist * 0.5, -MIN_STEP_DIST, MAX_STEP_DIST);
        rayWo = oriRayWo + rayWd * t;
        if (shadow < 0.0) {
            break;
        }
        if (distance(rayWo, oriRayWo) > maxTraceDist) {
            break;
        }
    }
    shadow = max(shadow, 0.0);
    return (shadow * shadow * (3.0 - 2.0 * shadow)); //smoothstep function from 0 to 1
}

