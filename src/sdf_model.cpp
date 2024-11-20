//
// Created by Alether on 5/4/2024.
//

#include "sdf_model.h"
#include "data/model.h"
#include "src/data/boundingbox.h"
#include "src/data/shader.h"
#include "src/data/transform.h"
#include "src/file_system.h"
#include <fstream>
#include <functional>
#include <iostream>

#include "texture.h"
#include "util.h"

using namespace ale;
using afs = ale::FileSystem;

SdfModel::SdfModel(Model &model, int cubeCount)
    : cubeCount(cubeCount), outerBB(model.meshes[0].boundingBox),
      bb(model.meshes[0].boundingBox) {
  // explode the bounding box a little bit
  Transform scaleBB{
      .scale = vec3(1.1, 1.1, 1.1),
  }; // make it a bit bigger
  this->outerBB = this->outerBB.applyTransform(scaleBB);

  cubeSize = vec3((outerBB.max.x - outerBB.min.x) / cubeCount,
                  (outerBB.max.y - outerBB.min.y) / cubeCount,
                  (outerBB.max.z - outerBB.min.z) / cubeCount);
  distances = vector(cubeCount, vector(cubeCount, vector(cubeCount, INFINITY)));
  positions = vector(cubeCount, vector(cubeCount, vector(cubeCount, vec3())));

  this->loopOverCubes([&](int k, int j, int i, BoundingBox bb) {
    vector<vec3> isectPoint;
    for (int tri = 0; tri + 2 < model.meshes[0].indices.size(); tri += 3) {
      Vertex a = model.meshes[0].vertices[model.meshes[0].indices[tri]];
      Vertex b = model.meshes[0].vertices[model.meshes[0].indices[tri + 1]];
      Vertex c = model.meshes[0].vertices[model.meshes[0].indices[tri + 2]];

      float tIsect;
      if (Util::rayTriangleIntersect(bb.center, normalize(vec3(0.0, 1.0, 0.0)),
                                     a.Position, b.Position, c.Position,
                                     tIsect)) {
        vec3 isect = bb.center + vec3(0.0, 1.0, 0.0) * tIsect;
        bool ok = true;
        for (auto ii : isectPoint) {
          if (distance(isect, ii) < 0.001) {
            ok = false;
            break;
          }
        }
        if (ok) {
          isectPoint.push_back(isect);
        }
      }

      float distance =
          Util::udTriangle(bb.center, a.Position, b.Position, c.Position);
      if (distance < distances[i][j][k]) {
        distances[i][j][k] = distance;
        positions[k][j][i] = bb.center;
      }
    }
    if (isectPoint.size() % 2 == 1) {
      distances[i][j][k] = -distances[i][j][k];
    }
  });

  if (distances.empty() || distances[0].empty() || distances[0][0].empty()) {
    throw runtime_error("unexpected emptiness in sdf distances");
  }

  vector<float> distances1D;
  for (int i = 0; i < cubeCount; ++i) {
    for (int j = 0; j < cubeCount; ++j) {
      for (int k = 0; k < cubeCount; ++k) {
        distances1D.push_back(distances[i][j][k]);
      }
    }
  }

  this->texture3D = Texture3D(Texture3D::Meta{.width = cubeCount,
                                              .height = cubeCount,
                                              .depth = cubeCount,
                                              .internal_format = GL_R32F,
                                              .input_format = GL_RED,
                                              .input_type = GL_FLOAT},
                              distances1D);
}

ale::SdfModel::SdfModel(Model &model, Texture3D texture3D, int cubeCount)
    : texture3D(std::move(texture3D)), cubeCount(cubeCount),
      outerBB(model.meshes[0].boundingBox), bb(model.meshes[0].boundingBox) {
  Transform scaleBB{
      .scale = vec3(1.1, 1.1, 1.1),
  }; // make it a bit bigger
  this->outerBB = this->outerBB.applyTransform(scaleBB);

  cubeSize = vec3((outerBB.max.x - outerBB.min.x) / cubeCount,
                  (outerBB.max.y - outerBB.min.y) / cubeCount,
                  (outerBB.max.z - outerBB.min.z) / cubeCount);
}

void SdfModel::loopOverCubes(function<void(int, int, int, BoundingBox)> func) {
  // 1. Generate the coordinates for each point that we have inside the bounding
  // box
  // 2. Loop over all the triangles in the mesh (with/without EBO)
  // for(int i = 0; i < cubeCount; ++i){
  //     float xOffset = cubeSize.x * i;
  //     for(int j = 0; j < cubeCount; ++j) {
  //         float yOffset = cubeSize.y * j;
  //         for(int k = 0; k < cubeCount; ++k) {
  //             float zOffset = cubeSize.z * k;
  //             vec3 currentPos = cubeStartPos + vec3(xOffset, yOffset,
  //             zOffset); BoundingBox bb = BoundingBox(
  //                 currentPos,
  //                 currentPos + cubeSize
  //             );
  //             func(Transform{.translation = currentPos}, bb);
  //         }
  //     }
  // }
  vec3 startPos = this->outerBB.min;
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
  if (texture3D.has_value()) {
    vec3 outerSize = this->outerBB.max - this->outerBB.min;
    shader.setVec3("outerBBMin", this->outerBB.min);
    shader.setVec3("outerBBMax", this->outerBB.max);

    vec3 innerSize = this->bb.max - this->bb.min;
    shader.setVec3("innerBBMin", this->bb.min);
    shader.setVec3("innerBBMax", this->bb.max);

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(shader.ID, "texture3D"), 0);
    glBindTexture(GL_TEXTURE_3D, this->texture3D->id);
  }
}

void SdfModel::writeToFile(string path) {
  ofstream outFile;
  outFile.open(afs::root(path));

  if (outFile.is_open()) {
    for (int i = 0; i < cubeCount; ++i) {
      outFile << "i: " << i << endl;
      for (int j = 0; j < cubeCount; ++j) {
        for (int k = 0; k < cubeCount; ++k) {
          outFile << distances[i][j][k] << " ";
        }
        outFile << "| j: " << j << endl;
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

bool SdfModel::findHitPositions(Ray debugRay, vector<vec3> *debugHitPos) {
  // to box only
  // cout << "hit T" << endl;
  // for(int i = 0; i < 20; ++i) {
  //     // first check the box.
  //     float dist = Util::distanceFromBox(debugRay.origin, this->outerBB.min,
  //     this->outerBB.max); debugRay.origin = debugRay.resolveT(dist);
  //     hitPos.push_back(debugRay.origin);
  //     cout << debugRay.origin.x << " " << debugRay.origin.y << " " <<
  //     debugRay.origin.z << endl;
  // }

  // to box with sdf
  // cout << "hit T" << endl;
  for (int i = 0; i < 20; ++i) {
    if (this->bb.isInside(debugRay.origin)) {
      vec3 localCoord = debugRay.origin - this->outerBB.min;
      vec3 boxArrSize = this->outerBB.getSize() / vec3(this->cubeCount);
      int x = localCoord.x / boxArrSize.x;
      int y = localCoord.y / boxArrSize.y;
      int z = localCoord.z / boxArrSize.z;
      float dist = this->distances[x][y][z];
      debugRay.origin = debugRay.resolveT(dist);

      if (dist < 0.01) {
        if (debugHitPos != nullptr) {
          debugHitPos->push_back(debugRay.origin);
          cout << "isect found: " << debugRay.origin.x << " "
               << debugRay.origin.y << " " << debugRay.origin.z << endl;
        }
        return true;
      }
    } else {
      float dist =
          Util::distanceFromBox(debugRay.origin, this->bb.min, this->bb.max);
      debugRay.origin = debugRay.resolveT(dist);
    }

    // first check the box.
    if (debugHitPos != nullptr) {
      debugHitPos->push_back(debugRay.origin);
      // cout << debugRay.origin.x << " " << debugRay.origin.y << " " <<
      // debugRay.origin.z << endl;
    }
  }
  if (debugHitPos != nullptr) {
    cout << "isect not found" << endl;
  }
  return false;
}

// Texture3D::Texture3D(vector<vector<vector<float>>> distances) {
//     if (distances.empty() || distances[0].empty() ||
//     distances[0][0].empty()){
//         return;
//     }
//
//     int width = distances.size();
//     int height = distances[0].size();
//     int depth = distances[0][0].size();
//
//     // it seems that when we pass to shader, they can only receive 1d array?
//     vector<float> distances1D;
//     for (int i = 0; i < width; ++i){
//         for (int j = 0; j < height; ++j){
//             for (int k = 0; k < depth; ++k){
//                 distances1D.push_back(distances[i][j][k]);
//             }
//         }
//     }
//
//     glGenTextures(1, &this->id);
//     glBindTexture(GL_TEXTURE_3D, this->id);
//     glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, width, height, depth, 0, GL_RED,
//     GL_FLOAT, distances1D.data());
//
//     glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//     glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//     glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
//     glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//     glBindTexture(GL_TEXTURE_3D, 0);
// }
