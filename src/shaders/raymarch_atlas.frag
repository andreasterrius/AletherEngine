#version 430 core

out vec4 out_color;

uniform vec2 iResolution;
uniform mat4 invViewProj;
uniform vec3 cameraPos;

uniform sampler2D atlas[16];
uniform int atlasSize;

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

vec2 ConvertWorldToTexture(vec3 worldPos, vec3 boxMin, vec3 boxSize)
{
    // vec3 texturePos3D = (worldPos - boxMin) / boxSize;
    float textureWidth = 4096;
    float textureHeight = 256;

    int cubeCount = 64;
    vec3 texturePos3D = (worldPos - boxMin) / (boxSize / vec3(cubeCount));
    float x = texturePos3D.x;
    float y = texturePos3D.y;
    float z = floor(texturePos3D.z);

    vec2 texturePos2D = vec2(
        (x / textureWidth) + (z * 64.0 / textureWidth),
        (y / textureHeight) // Y coordinate
    );

    return texturePos2D;
}

float distance_from_texture3D(vec3 p, int atlasIndex, int atlasOffset, vec3 outerBBMin, vec3 outerBBMax)
{
    // vec3 uvwCoord = ConvertWorldToTexture(p, outerBBMin, outerBBMax-outerBBMin);
    // vec2 uvCoord = vec2(uvwCoord.x + uvwCoord.z*64, (atlasOffset*64)+(uvwCoord.y/256.0));
    vec2 uvCoord = ConvertWorldToTexture(p, outerBBMin, outerBBMax-outerBBMin);
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

float sdBox( vec3 p, vec3 b )
{
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

bool is_inside_box(vec3 p, vec3 bbMin, vec3 bbMax) {
    float tolerance = 0.001;
    return (bbMin.x - tolerance <= p.x && p.x <= bbMax.x + tolerance &&
            bbMin.y - tolerance <= p.y && p.y <= bbMax.y + tolerance &&
            bbMin.z - tolerance <= p.z && p.z <= bbMax.z + tolerance
    );
}

vec3 lambertBRDF(vec3 normal, vec3 lightDir, vec3 albedo) {
    float NdotL = max(dot(normal, lightDir), 0.0);
    return albedo * NdotL / 3.14;
}

vec3 raymarch(vec3 rayWo, vec3 rayWd) {
    float total_distance_traveled = 0.0;
    const int NUMBER_OF_STEPS = 64;
    const float MINIMUM_HIT_DISTANCE = 0.01;
    const float MAXIMUM_TRACE_DISTANCE = 10000.0;
    const vec3 NO_HIT_COLOR = vec3(0.52, 0.8, 0.92);
    const vec3 SDF_COLOR =  vec3(0.89, 0.89, 0.56);

    // make a light because why not
    const vec3 lightPos = vec3(3.0, 5.0, 5.0);
    const vec3 lightColor = vec3(3.0, 3.0, 3.0);
   
    for (int i = 0; i < NUMBER_OF_STEPS; ++i)
    {
        float closestDist = 1000000; 
        int closestDistIndex = -1;
        vec3 rayLo = vec3(0.0);
        vec3 rayLd = vec3(0.0);
        for(int j = 0; j < offsetSize; ++j)
        {
            mat4 invModelMat = offsets[j].invModelMat;
            vec3 innerBBMin = vec3(offsets[j].innerBBMin);
            vec3 innerBBMax = vec3(offsets[j].innerBBMax);
            vec3 outerBBMin = vec3(offsets[j].outerBBMin);
            vec3 outerBBMax = vec3(offsets[j].outerBBMax);
            int atlasIndex = offsets[j].atlasIndex;
            int atlasOffset = offsets[j].atlasOffset;

            rayLo = vec3(invModelMat * vec4(rayWo, 1.0));
            rayLd = vec3(normalize(invModelMat * vec4(rayWd, 0.0)));

            float dist = distance_from_box_minmax(rayLo, innerBBMin, innerBBMax);
            float outerDist = distance_from_box_minmax(rayLo, outerBBMin, outerBBMax);
            if(outerDist < 0.0) {
                dist = distance_from_texture3D(rayLo, atlasIndex, atlasOffset, outerBBMin, outerBBMax);
            }
            // if(dist < 0.0) {
            //     dist = distance_from_texture3D(rayLo, atlasIndex, atlasOffset, outerBBMin, outerBBMax);
            // }
            // if (!is_inside_box(rayLo, outerBBMin, outerBBMax)) {
            //     dist = distance_from_box_minmax(rayLo, outerBBMin, outerBBMax);
            // } else {
            //     dist = distance_from_texture3D(rayLo, atlasIndex, atlasOffset, outerBBMin, outerBBMax);
            //     return vec3(distance_from_texture3D(rayLo, atlasIndex, atlasOffset, outerBBMin, outerBBMax));
            //      return vec3(ConvertWorldToTexture(rayLo, outerBBMin, outerBBMax-outerBBMin), 0.0);
            //     if(dist < MINIMUM_HIT_DISTANCE)
            //     {
            //         return vec3(0.0, 1.0, 0.0);
            //         // vec3 normal = normalize(ro - vec3(offsets[closestDistIndex].innerBBMin + offsets[closestDistIndex].innerBBMax/2.0));
            //         // return lambertBRDF(normal, normalize(lightPos - ro), SDF_COLOR);
            //     }
            // }
            
            if(dist < closestDist) {
                closestDist = dist;
                closestDistIndex = j;
            }
        }
        rayWo = vec3(offsets[closestDistIndex].modelMat * vec4(rayLo+rayLd*closestDist, 1.0));
        if(closestDist > MAXIMUM_TRACE_DISTANCE) {
            return NO_HIT_COLOR;
        }
        if(closestDist < MINIMUM_HIT_DISTANCE)
        {
            vec3 isectPos = rayWo;
            vec3 innerBBMin = vec3(offsets[closestDistIndex].innerBBMin);
            vec3 innerBBMax = vec3(offsets[closestDistIndex].innerBBMax);
            vec3 normal = normalize(isectPos - (innerBBMin + innerBBMax/2.0));
            return lambertBRDF(normal, normalize(lightPos - isectPos), SDF_COLOR);
            // vec3 normal = normalize(ro - vec3(offsets[closestDistIndex].innerBBMin + offsets[closestDistIndex].innerBBMax/2.0));
            // return lambertBRDF(normal, normalize(lightPos - ro), SDF_COLOR);
        }
    }
    return NO_HIT_COLOR;
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
    // color = pow(color, vec3(1.0 / 2.2));

    // vec3 color = vec3(1.0);
    out_color = vec4(color, 1.0);
}
