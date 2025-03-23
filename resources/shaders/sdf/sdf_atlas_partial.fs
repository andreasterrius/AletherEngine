// SSBO, so 430 core is required

uniform sampler2D atlas[6];
uniform int atlasSize;
uniform int atlasStartIndex;

struct PackedSdfOffsetDetail {
    mat4 modelMat;
    mat4 invModelMat;
    vec4 innerBBMin;
    vec4 innerBBMax;
    vec4 outerBBMin;
    vec4 outerBBMax;
    int atlasIndex;
    int atlasOffset;
};

layout (std430, binding = 0) buffer PackedSdfOffsetDetailBuffer {
    int offsetSize;
    PackedSdfOffsetDetail offsets[];
};

vec2 convert_world_to_texture(vec3 worldPos, vec3 boxMin, vec3 boxSize, int atlasOffset)
{
    float textureWidth = 4096;
    float textureHeight = 4096;

    int cubeCount = 64;
    vec3 texturePos3D = (worldPos - boxMin) / (boxSize / vec3(cubeCount));
//    float x = clamp(texturePos3D.x, 0, 63);
//    float y = clamp(texturePos3D.y, 0, 63);
//    float z = clamp(floor(texturePos3D.z), 0, 63);
    float x = texturePos3D.x;
    float y = texturePos3D.y;
    float z = floor(texturePos3D.z);

    vec2 texturePos2D = vec2(
        (x / textureWidth) + (z * 64.0 / textureWidth),
        (y / textureHeight) + (atlasOffset * 64.0 / textureHeight)// Y coordinate
    );

    return texturePos2D;
}

float distance_from_texture3D(vec3 p, int atlasIndex, int atlasOffset, vec3 outerBBMin, vec3 outerBBMax)
{
    vec2 uvCoord = convert_world_to_texture(p, outerBBMin, outerBBMax-outerBBMin, atlasOffset);
    return texture(atlas[atlasIndex], uvCoord).r;
}

float distance_from_box_minmax(vec3 p, vec3 bbMin, vec3 bbMax) {
    // Calculate the distance to the box surface along each axis
    vec3 d = max(p - bbMax, bbMin - p);

    // If all components of d are <= 0, the point is inside the box.
    float outsideDist = length(max(d, 0.0));  // Distance outside the box
    float insideDist = min(max(d.x, max(d.y, d.z)), 0.0);  // Negative inside distance
    // float insideDist = 0.0;

    // Return the signed distance: negative if inside, positive if outside
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
    const float k = 8;
    float t = 0.0;
    float shadow = 1.0;

    for (int i = 0; i < NUMBER_OF_STEPS; ++i)
    {
        float shadowDist = 100000;
        float closestDist = 1000000;
        int closestDistIndex = -1;
        for(int j = 0; j < offsetSize; ++j)
        {
            mat4 invModelMat = offsets[j].invModelMat;
            vec3 innerBBMin = vec3(offsets[j].innerBBMin);
            vec3 innerBBMax = vec3(offsets[j].innerBBMax);
            vec3 outerBBMin = vec3(offsets[j].outerBBMin);
            vec3 outerBBMax = vec3(offsets[j].outerBBMax);
            int atlasIndex = offsets[j].atlasIndex;
            int atlasOffset = offsets[j].atlasOffset;
            float scaleFactor = get_scale_factor(invModelMat, rayWd);

            vec3 rayLo = vec3(invModelMat * vec4(rayWo, 1.0));
            vec3 rayLd = vec3(normalize(invModelMat * vec4(rayWd, 0.0)));

            float dist = distance_from_box_minmax(rayLo, innerBBMin, innerBBMax);
            float outerDist = distance_from_box_minmax(rayLo, outerBBMin, outerBBMax);
            if(outerDist < 0.0) {
                // inside the sdf
                dist = distance_from_texture3D(rayLo, atlasIndex, atlasOffset, outerBBMin, outerBBMax);
            }

            // transform dist to world space
            dist = dist / scaleFactor;
            if(dist < closestDist) {
                if(outerDist < 0.0) {
                    shadowDist = dist;
                }
                closestDist = dist;
                closestDistIndex = j;
            }
        }

        shadow = min(shadow, shadowDist*k/(t+0.0001));
        t += clamp(closestDist * 0.5, MIN_STEP_DIST, MAX_STEP_DIST);
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

