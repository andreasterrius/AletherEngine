#version 330 core

out vec4 out_color;

uniform vec2 iResolution;
uniform float iTime;
uniform mat4 invViewProj;
uniform vec3 cameraPos;

uniform vec3 outerBBMin;
uniform vec3 outerBBMax;
uniform vec3 outerBBSize;

uniform vec3 innerBBMin;
uniform vec3 innerBBMax;
uniform vec3 innerBBSize;

uniform vec3 textureSize;
uniform sampler3D texture3D;

vec3 ConvertWorldToTexture(vec3 worldPos, vec3 boxMin, vec3 boxSize)
{
    vec3 texturePos = (worldPos - boxMin) / boxSize;
    return texturePos;
}

float distance_from_texture3D(vec3 p)
{
    vec3 uvwCoord = ConvertWorldToTexture(p, outerBBMin, outerBBSize);
    return texture(texture3D, uvwCoord).r;
}

float distance_from_sphere(in vec3 p, in vec3 c, float r)
{
    return length(p - c) - r;
}

float distance_from_box(vec3 p, vec3 size) {
    vec3 q = abs(p) - size / 2.0f;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

float distance_from_box_minmax(vec3 p, vec3 bbMin, vec3 bbMax) {
    // Calculate the distance to the box surface along each axis
    vec3 d = max(p - bbMax, bbMin - p);

    // If all components of d are <= 0, the point is inside the box.
    float outsideDist = length(max(d, 0.0));  // Distance outside the box
    float insideDist = min(max(d.x, max(d.y, d.z)), 0.0);  // Negative inside distance

    // Return the signed distance: negative if inside, positive if outside
    return outsideDist + insideDist;
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

vec3 raymarch(vec3 ro, vec3 rd) {
    float total_distance_traveled = 0.0;
    const int NUMBER_OF_STEPS = 100;
    const float MINIMUM_HIT_DISTANCE = 0.01;
    const float MAXIMUM_TRACE_DISTANCE = 1000.0;
    const vec3 NO_HIT_COLOR = vec3(0.52, 0.8, 0.92);
    const vec3 SDF_COLOR =  vec3(0.83, 0.3, 0.05);

    // make a light because why not
    const vec3 lightPos = vec3(3.0, 5.0, 5.0);
    const vec3 lightColor = vec3(3.0, 3.0, 3.0);

    for (int i = 0; i < NUMBER_OF_STEPS; ++i)
    {
        if (!is_inside_box(ro, innerBBMin, innerBBMax)) {
            float dist = distance_from_box_minmax(ro, innerBBMin, innerBBMax);
            ro = ro + rd * dist;

            if (dist > MAXIMUM_TRACE_DISTANCE) {
                return NO_HIT_COLOR;
            }

        } else {
            float dist = distance_from_texture3D(ro);
//            // CHECK TEXTURE UVW
//            return vec3(ConvertWorldToTexture(ro, outerBBMin, outerBBMax));
            if (dist < MINIMUM_HIT_DISTANCE) {
                vec3 isectPos = ro+rd*dist;
                vec3 normal = normalize(isectPos - (innerBBMin + innerBBMax/2.0));
                return lambertBRDF(normal, normalize(lightPos - isectPos), SDF_COLOR);
            }
            ro = ro + rd * dist;
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
    out_color = vec4(color, 1.0);
}