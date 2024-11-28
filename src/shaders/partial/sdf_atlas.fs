// SSBO, so 430 core is required

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

layout (std430, binding = <0>) buffer PackedSdfOffsetDetailBuffer {
    int offsetSize;
    PackedSdfOffsetDetail offsets[];
};

vec2 convert_world_to_texture(vec3 worldPos, vec3 boxMin, vec3 boxSize, int atlasOffset)
{
    float textureWidth = 4096;
    float textureHeight = 256;

    int cubeCount = 64;
    vec3 texturePos3D = (worldPos - boxMin) / (boxSize / vec3(cubeCount));
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
