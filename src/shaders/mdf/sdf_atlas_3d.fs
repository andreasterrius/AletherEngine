
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

bool raymarch(vec3 rayWo, vec3 rayWd, float maxTraceDist, out vec3 isectPos, out vec3 objectCenter) {

    const int NUMBER_OF_STEPS = 100;
    const float MINIMUM_HIT_DISTANCE = 0.01;
    const float MAXIMUM_TRACE_DISTANCE = maxTraceDist;
    const vec3 NO_HIT_COLOR = vec3(0.52, 0.8, 0.92);
    const vec3 SDF_COLOR =  vec3(0.89, 0.89, 0.56);
    const vec3 oriRayWo = rayWo;

    for (int i = 0; i < NUMBER_OF_STEPS; ++i)
    {
        vec3 rayLo = vec3(mdf.invModelMat * vec4(rayWo, 1.0));
        vec3 rayLd = vec3(normalize(mdf.invModelMat * vec4(rayWd, 0.0)));

        float dist = distance_from_box_minmax(rayLo, mdf.innerBBMin, mdf.innerBBMax);
        float outerDist = distance_from_box_minmax(rayLo, mdf.outerBBMin, mdf.outerBBMax);
        if(outerDist < 0.0) {
            dist = distance_from_texture3D(rayLo, mdf.outerBBMin, mdf.outerBBMax);
        }

        rayWo = vec3(mdf.modelMat * vec4(rayLo+rayLd*dist, 1.0));
        if(distance(rayWo, oriRayWo) > MAXIMUM_TRACE_DISTANCE) {
            return false;
        }
        if(dist < MINIMUM_HIT_DISTANCE)
        {
            isectPos = rayWo;
            objectCenter = (vec3(mdf.innerBBMin) + vec3(mdf.innerBBMax))/2.0;
            return true;
        }
    }
    return false;
}

