// #version 430 core

// out vec4 out_color;

// uniform vec2 iResolution;
// uniform mat4 invViewProj;
// uniform vec3 cameraPos;

// struct PackedSdfOffsetDetail {
//     mat4 modelMat;
//     mat4 invModelMat;
//     vec4 innerBBMin;
//     vec4 innerBBMax;
//     vec4 outerBBMin;
//     vec4 outerBBMax;
//     int atlasIndex;
//     int atlasCount;
// };

// layout (std430, binding = 2) buffer PackedSdfOffsetDetailBuffer {
//     int offset_size;
//     PackedSdfOffsetDetail offsets[];
// };

// uniform sampler2D packedSdf[16];
// uniform int packedSdfSize;

// vec3 convert_world_to_atlas(vec3 world_pos, int atlas_offset, vec3 box_min, vec3 box_size)
// {
//     vec3 uvw_coord = (world_pos - box_min) / box_size;
//     return vec2(uvw_coord.x + uvw_coord.z*64, (offset*64)+uvw_coord.y);
// }

// float distance_from_sdf_atlas(vec3 world_pos, sampler2D atlas, 
//     int atlas_offset, vec3 outer_bb_min, vec3 outer_bb_max) 
// {
//     vec2 uv_coord = convert_world_to_atlas(world_pos, atlas_offset, 
//         outer_bb_min, outer_bb_max-outer_bb_min);
//     return texture(packedSdf[0], uv_coord).r; 
// }

// // float distance_from_sphere(in vec3 p, in vec3 c, float r)
// // {
// //     return length(p - c) - r;
// // }

// float distance_from_box(vec3 p, vec3 size) {
//     vec3 q = abs(p) - size / 2.0f;
//     return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
// }

// float distance_from_box_minmax(vec3 p, vec3 bbMin, vec3 bbMax) {
//     // Calculate the distance to the box surface along each axis
//     vec3 d = max(p - bbMax, bbMin - p);

//     // If all components of d are <= 0, the point is inside the box.
//     float outsideDist = length(max(d, 0.0));  // Distance outside the box
//     float insideDist = min(max(d.x, max(d.y, d.z)), 0.0);  // Negative inside distance

//     // Return the signed distance: negative if inside, positive if outside
//     return outsideDist + insideDist;
// }

// bool is_inside_box(vec3 p, vec3 bbMin, vec3 bbMax) {
//     float tolerance = 0.001;
//     return (bbMin.x - tolerance <= p.x && p.x <= bbMax.x + tolerance &&
//             bbMin.y - tolerance <= p.y && p.y <= bbMax.y + tolerance &&
//             bbMin.z - tolerance <= p.z && p.z <= bbMax.z + tolerance
//     );
// }

// vec3 lambertBRDF(vec3 normal, vec3 lightDir, vec3 albedo) {
//     float NdotL = max(dot(normal, lightDir), 0.0);
//     return albedo * NdotL / 3.14;
// }

// vec3 raymarch(vec3 ro, vec3 rd) {
//     float total_distance_traveled = 0.0;
//     const int NUMBER_OF_STEPS = 100;
//     const float MINIMUM_HIT_DISTANCE = 0.01;
//     const float MAXIMUM_TRACE_DISTANCE = 1000.0;
//     const vec3 NO_HIT_COLOR = vec3(0.52, 0.8, 0.92);
//     const vec3 SDF_COLOR =  vec3(0.83, 0.3, 0.05);

//     // make a light because why not
//     const vec3 lightPos = vec3(3.0, 5.0, 5.0);
//     const vec3 lightColor = vec3(3.0, 3.0, 3.0);

//     ro = vec3(invModelMat * vec4(ro, 1.0));
//     rd = vec3(normalize(invModelMat * vec4(rd, 0.0))); 

//     for (int i = 0; i < NUMBER_OF_STEPS; ++i)
//     {
//         if (!is_inside_box(ro, innerBBMin, innerBBMax)) {
//             float dist = distance_from_box_minmax(ro, innerBBMin, innerBBMax);
//             ro = ro + rd * dist;

//             if (dist > MAXIMUM_TRACE_DISTANCE) {
//                 return NO_HIT_COLOR;
//             }

//         } else {
//             float dist = distance_from_texture3D(ro);
// //            // CHECK TEXTURE UVW
// //            return vec3(ConvertWorldToTexture(ro, outerBBMin, outerBBMax));
//             if (dist < MINIMUM_HIT_DISTANCE) {
//                 vec3 isectPos = vec3(modelMat*vec4(ro+rd*dist, 1.0));
//                 vec3 normal = normalize(isectPos - (innerBBMin + innerBBMax/2.0));
//                 return lambertBRDF(normal, normalize(lightPos - isectPos), SDF_COLOR);
//             }
//             ro = ro + rd * dist;
//         }
//     }
//     return NO_HIT_COLOR;
// }

// void main()
// {
//     // Current point we're hitting
//     vec2 uv = gl_FragCoord.xy / iResolution.xy * 2.0 - 1.0;

//     vec4 rayStartWorld = vec4(cameraPos, 1.0);
//     vec4 rayEndWorld = invViewProj * vec4(uv, 0.0, 1.0);
//     rayEndWorld /= rayEndWorld.w;

//     vec3 rayDir = vec3(normalize(rayEndWorld - rayStartWorld));

//     vec3 color = raymarch(vec3(rayStartWorld), rayDir);
//     out_color = vec4(color, 1.0);
// }
