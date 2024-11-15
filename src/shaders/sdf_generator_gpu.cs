#version 430 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout (r32f, binding = 0) uniform image3D imgOutput;
layout (rgba32f, binding = 1) uniform image2D debugResult;

struct Vertex {
    vec4 position;
    vec4 normal;
    vec2 texCoords;
    vec4 tangent;
    vec4 bitangent;
    ivec4 bone_ids;
    vec4 weights;
};
layout (std430, binding = 2) buffer VertexBuffer {
    uint vertices_size;
    Vertex vertices[];
};

layout (std430, binding = 3) buffer IndexBuffer {
    uint indices_size;
    uint indices[];
};

layout (std140, binding = 4) uniform BoundingBox {
    vec4 inner_bb_min;
    vec4 inner_bb_max;
    vec4 outer_bb_min;
    vec4 outer_bb_max;
};

float dot2(vec3 v) { return dot(v,v); }

float udTriangle( vec3 p, vec3 a, vec3 b, vec3 c ){
    vec3 ba = b - a; vec3 pa = p - a;
    vec3 cb = c - b; vec3 pb = p - b;
    vec3 ac = a - c; vec3 pc = p - c;
    vec3 nor = cross( ba, ac );

    return sqrt(
      (sign(dot(cross(ba,nor),pa)) +
       sign(dot(cross(cb,nor),pb)) +
       sign(dot(cross(ac,nor),pc))<2.0)
       ?
       min( min(
       dot2(ba*clamp(dot(ba,pa)/dot2(ba),0.0f,1.0f)-pa),
       dot2(cb*clamp(dot(cb,pb)/dot2(cb),0.0f,1.0f)-pb) ),
       dot2(ac*clamp(dot(ac,pc)/dot2(ac),0.0f,1.0f)-pc) )
       :
       dot(nor,pa)*dot(nor,pa)/dot2(nor) );
}

bool epsilonEqual(float x, float y, float epsilon){
    return abs(x - y) < epsilon;
}

bool rayTriangleIntersect(vec3 orig, vec3 dir, vec3 v0, vec3 v1, vec3 v2, inout float t) {
    float EPSILON = 1e-6f;
    vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;
    vec3 h = cross(dir, edge2);
    float a = dot(edge1, h);
    if (epsilonEqual(a, 0.0f, EPSILON)) {
        return false; // Ray is parallel to the triangle
    }
    float f = 1.0f / a;
    vec3 s = orig - v0;
    float u = f * dot(s, h);
    if (u < 0.0f || u > 1.0f) {
        return false;
    }
    vec3 q = cross(s, edge1);
    float v = f * dot(dir, q);
    if (v < 0.0f || u + v > 1.0f) {
        return false;
    }
    // At this stage, we can compute t to find out where the intersection point is on the line
    t = f * dot(edge2, q);
    if (t > EPSILON) { // Ray intersection
        return true;
    } else { // Line intersection but not a ray intersection
        return false;
    }
}

void main() {
    ivec3 texel_coord = ivec3(gl_GlobalInvocationID.xyz);
    ivec3 image_size = imageSize(imgOutput);
    vec3 color = vec3(0.0, 0.0, 0.0);

    vec3 cube_size = vec3(
        (outer_bb_max.x - outer_bb_min.x) / image_size.x,
        (outer_bb_max.y - outer_bb_min.y) / image_size.y,
        (outer_bb_max.z - outer_bb_min.z) / image_size.z
    );
    vec3 cube_center_pos = (outer_bb_min.xyz+cube_size/2) + cube_size * texel_coord; 

    // find distance to all triangles
    float final_distance = 1000.0; //very large number
    vec3 isectPoint[100];
    int isectPointSize = 0;
    for(int i = 0; i < indices_size; i+=3){
        Vertex a = vertices[indices[i]];
        Vertex b = vertices[indices[i+1]];
        Vertex c = vertices[indices[i+2]];

        float tIsect = 0.0f;
        if (rayTriangleIntersect(cube_center_pos, normalize(vec3(0.0, 1.0, 0.0)),
                                        a.position.xyz, b.position.xyz, c.position.xyz, tIsect)){
            vec3 isect = cube_center_pos + vec3(0.0, 1.0, 0.0) * tIsect;

            // check if we have already intersected this before
            bool ok = true;
            for(int j = 0; j < isectPointSize; j++) {
                if(distance(isect, isectPoint[j]) < 1e-6f) {
                    ok = false;
                    break;
                }
            }
            if(ok) {
                if(isectPointSize < 100) {
                    isectPointSize++;
                }
                isectPoint[isectPointSize-1] = isect;
            }
        }

        float curr_distance = udTriangle(cube_center_pos, a.position.xyz, b.position.xyz, c.position.xyz);
        if(curr_distance < final_distance) {
            final_distance = curr_distance;
        }
    }
    if (isectPointSize % 2 == 1){
        final_distance = -final_distance;
    }
    
    vec3 debug_color = vec3(final_distance, 0.0, 0.0);
    uint debug_index = gl_GlobalInvocationID.x + 
                        gl_GlobalInvocationID.y * gl_NumWorkGroups.x + 
                        gl_GlobalInvocationID.z * gl_NumWorkGroups.x * gl_NumWorkGroups.y;
    // if(debug_index < vertices_size) {
    //     debug_color = vec3(vertices[debug_index].normal.xyz);
    // }
    imageStore(debugResult, ivec2(debug_index, 0), vec4(debug_color, 1.0));

    // texel_coord.x = image_size.x - texel_coord.x;
    imageStore(imgOutput, texel_coord, vec4(final_distance, 0.0, 0.0, 1.0));
}
