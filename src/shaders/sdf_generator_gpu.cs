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
float udTriangle( vec3 p, vec3 a, vec3 b, vec3 c )
{
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

void main() {
    ivec3 texel_coord = ivec3(gl_GlobalInvocationID.xyz);
    ivec3 image_size = imageSize(imgOutput);
    vec3 color = vec3(0.0, 0.0, 0.0);

    vec3 cube_size = vec3(
        (outer_bb_max.x - outer_bb_min.x) / image_size.x,
        (outer_bb_max.y - outer_bb_min.y) / image_size.y,
        (outer_bb_max.z - outer_bb_min.z) / image_size.z
    );
    vec3 cube_center_pos = outer_bb_min.xyz + cube_size * texel_coord; 

    // find distance to all triangles
    float distance = 1000.0; //very large number
    for(int i = 0; i < indices_size; i+=3){
        Vertex a = vertices[indices[i]];
        Vertex b = vertices[indices[i+1]];
        Vertex c = vertices[indices[i+2]];

        float curr_distance = udTriangle(cube_center_pos, a.position.xyz, b.position.xyz, c.position.xyz);
        if(curr_distance < distance) {
            distance = curr_distance;
        }
    }

    distance = 1.0;
    color = vec3(distance);

    imageStore(imgOutput, texel_coord, vec4(distance, 0.0, 0.0, 1.0));
    imageStore(debugResult, texel_coord.xy, vec4(color, 1.0));

    barrier();
}
