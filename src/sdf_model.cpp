//
// Created by Alether on 5/4/2024.
//

#include "sdf_model.h"
#include "data/model.h"
#include "src/data/transform.h"
#include "src/data/boundingbox.h"
#include "src/data/shader.h"
#include "src/file_system.h"
#include <functional>
#include <iostream>
#include <fstream>

#include "util.h"

using namespace ale;
using afs = ale::FileSystem;


SdfModel::SdfModel(Model &model, int cubeCount) : cubeCount(cubeCount), boundingBox(model.meshes[0].boundingBox) {
    // explode the bounding box a little bit
    Transform scaleBB{.scale = vec3(1.1, 1.1, 1.1),}; // make it a bit bigger
    this->boundingBox = this->boundingBox.applyTransform(scaleBB);

    cubeSize = vec3(
        (boundingBox.max.x - boundingBox.min.x) / cubeCount,
        (boundingBox.max.y - boundingBox.min.y) / cubeCount,
        (boundingBox.max.z - boundingBox.min.z) / cubeCount
    );
    distances = vector(cubeCount, vector(cubeCount, vector(cubeCount, INFINITY)));
    positions = vector(cubeCount, vector(cubeCount, vector(cubeCount, vec3())));

    this->loopOverCubes([&](int i, int j, int k, BoundingBox bb) {
        for (int tri = 0; tri + 2 < model.meshes[0].indices.size(); tri += 3) {
            Vertex a = model.meshes[0].vertices[model.meshes[0].indices[tri]];
            Vertex b = model.meshes[0].vertices[model.meshes[0].indices[tri + 1]];
            Vertex c = model.meshes[0].vertices[model.meshes[0].indices[tri + 2]];


            float distance = Util::udTriangle(bb.center, a.Position, b.Position, c.Position);
            if(distance < distances[i][j][k]) {
                distances[i][j][k] = distance;
                positions[i][j][k] = bb.center;
            }
        }
    });

    this->texture3D = Texture3D(distances);
}

void SdfModel::loopOverCubes(function<void(int, int, int, BoundingBox)> func) {
    // 1. Generate the coordinates for each point that we have inside the bounding box
    // 2. Loop over all the triangles in the mesh (with/without EBO)
    // for(int i = 0; i < cubeCount; ++i){
    //     float xOffset = cubeSize.x * i;
    //     for(int j = 0; j < cubeCount; ++j) {
    //         float yOffset = cubeSize.y * j;
    //         for(int k = 0; k < cubeCount; ++k) {
    //             float zOffset = cubeSize.z * k;
    //             vec3 currentPos = cubeStartPos + vec3(xOffset, yOffset, zOffset);
    //             BoundingBox bb = BoundingBox(
    //                 currentPos,
    //                 currentPos + cubeSize
    //             );
    //             func(Transform{.translation = currentPos}, bb);
    //         }
    //     }
    // }
    vec3 startPos = this->boundingBox.min;
    for (int i = 0; i < cubeCount; ++i) {
        float xOffset = cubeSize.x * i;
        for (int j = 0; j < cubeCount; ++j) {
            float yOffset = cubeSize.y * j;
            for (int k = 0; k < cubeCount; ++k) {
                float zOffset = cubeSize.z * k;
                vec3 pos = startPos + vec3(xOffset, yOffset, zOffset);
                func(i, j, k, BoundingBox(pos, pos + cubeSize));
            }
        }
    }
}

void SdfModel::bindToShader(Shader shader) {
    if(texture3D.has_value()) {

        vec3 size = this->boundingBox.max - this->boundingBox.min;
        shader.setVec3("bbMin", this->boundingBox.min);
        shader.setVec3("bbSize", size);
        shader.setVec3("textureSize", this->cubeSize);

        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(shader.ID, "texture3D"), 0);
        glBindTexture(GL_TEXTURE_2D, this->texture3D->id);
    }
}

void SdfModel::writeToFile(string path) {
    ofstream outFile;
    outFile.open(afs::root(path));

    if(outFile.is_open()) {
        for (int i = 0; i < cubeCount; ++i) {
            outFile << "i: " << i << endl;
            for (int j = 0; j < cubeCount; ++j) {
                for (int k = 0; k < cubeCount; ++k) {
                    outFile << distances[i][j][k] << " ";
                }
                outFile << endl;
            }
            outFile << endl;
        }
    } else {
        cout << "unable to write to " << path << endl;
        return;
    }

    outFile.close();
    cout << "data written to " << path << endl;
}

vector<vec3> SdfModel::findHitPositions(Ray debugRay) {
    vector<vec3> hitPos;
    //TODO: complete this
    return hitPos;
}


Texture3D::Texture3D(vector<vector<vector<float>>> distances) {

    if(distances.empty() || distances[0].empty() || distances[0][0].empty()) {
        return;
    }

    int width = distances.size();
    int height = distances[0].size();
    int depth = distances[0][0].size();

    // it seems that when we pass to shader, they can only receive 1d array?
    vector<float> distances1D;
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            for (int k = 0; k < depth; ++k) {
                distances1D.push_back(distances[i][j][k]);
            }
        }
    }

    glGenTextures(1, &this->id);
    glBindTexture(GL_TEXTURE_3D, this->id);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, width, height, depth, 0, GL_RED, GL_FLOAT, distances1D.data());

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_3D, 0);
}
