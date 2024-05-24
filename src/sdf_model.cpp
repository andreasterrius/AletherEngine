//
// Created by Alether on 5/4/2024.
//

#include "sdf_model.h"
#include "data/model.h"
#include "src/data/transform.h"
#include "src/data/boundingbox.h"
#include <functional>

using namespace ale;

SdfModel::SdfModel(Model &model, int cubeCount) : cubeCount(cubeCount), boundingBox(model.meshes[0].boundingBox){
    // explode the bounding box a little bit
    Transform scaleBB{.scale=vec3(1.1, 1.1, 1.1),}; // make it a bit bigger
    this->boundingBox = this->boundingBox.applyTransform(scaleBB);
//
//    distances = vector<vector<vector<float>>>(
//            cubeCount, vector<vector<float>>(
//                    cubeCount, vector<float>(
//                            cubeCount, 0.0f)));
//
//    // 1. Generate the coordinates for each point that we have inside the bounding box
//    cubeSize = vec3(
//            (model.meshes[0].boundingBox.max.x - model.meshes[0].boundingBox.min.x) / cubeCount,
//            (model.meshes[0].boundingBox.max.y - model.meshes[0].boundingBox.min.y) / cubeCount,
//            (model.meshes[0].boundingBox.max.z - model.meshes[0].boundingBox.min.z) / cubeCount
//    );
//    // find the starting point of the sdf volume
//    vec3 cubeHalfExtent = cubeSize / 2.0f;
//    vec3 cubeStartPos = this->boundingBox.min + cubeHalfExtent;
//
//    // 2. Loop over all the triangles in the mesh (with/without EBO)
//    for(int i = 0; i < cubeCount; ++i){
//        float xOffset = cubeSize.x * i;
//        for(int j = 0; j < cubeCount; ++j) {
//            float yOffset = cubeSize.y * j;
//            for(int k = 0; k < cubeCount; ++k) {
//                float zOffset = cubeSize.z * k;
//                vec3 currentPos = cubeStartPos + vec3(xOffset, yOffset, zOffset);
//                distances[i][j][k] = INFINITY;
//
//                // loop over all triangles
//                // 3. From the triangles, calculate the distance to the coordinate of (1.)
//                for(int tri = 0; tri+2 < model.meshes[0].indices.size(); tri += 3) {
//                    Vertex a = model.meshes[0].vertices[model.meshes[0].indices[tri]];
//                    Vertex b = model.meshes[0].vertices[model.meshes[0].indices[tri+1]];
//                    Vertex c = model.meshes[0].vertices[model.meshes[0].indices[tri+2]];
//
//                    // find the distance between the triangle and point
//                }
//            }
//        }
//    }
}

void SdfModel::loopOverCubes(function<void(Transform, BoundingBox)> func) {

    // 1. Generate the coordinates for each point that we have inside the bounding box
    cubeSize = vec3(
            (boundingBox.max.x - boundingBox.min.x) / cubeCount,
            (boundingBox.max.y - boundingBox.min.y) / cubeCount,
            (boundingBox.max.z - boundingBox.min.z) / cubeCount
    );
    // find the starting point of the sdf volume
    vec3 cubeHalfExtent = cubeSize / 2.0f;
    vec3 cubeStartPos = this->boundingBox.min + cubeHalfExtent;

    // 2. Loop over all the triangles in the mesh (with/without EBO)
    for(int i = 0; i < cubeCount; ++i){
        float xOffset = cubeSize.x * i;
        for(int j = 0; j < cubeCount; ++j) {
            float yOffset = cubeSize.y * j;
            for(int k = 0; k < cubeCount; ++k) {
                float zOffset = cubeSize.z * k;
                vec3 currentPos = cubeStartPos + vec3(xOffset, yOffset, zOffset);
                BoundingBox bb = BoundingBox(
                    currentPos - cubeHalfExtent,
                    currentPos + cubeHalfExtent
                );
                func(Transform{.translation = currentPos}, bb);
            }
        }
    }
}

