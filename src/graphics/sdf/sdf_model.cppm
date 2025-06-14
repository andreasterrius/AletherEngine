//
// Created by Alether on 5/4/2024.
//

module;

#include <fstream>
#include <functional>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include "src/data/util.h"

export module sdf_model;
import bounding_box;
import ray;
import texture;
import shader;
import mesh;
import model;
import transform;
import file_system;

using namespace std;
using namespace ale::data;
using namespace ale::graphics;
using afs = ale::FileSystem;
using namespace glm;

export namespace ale::graphics::sdf {

// This is a 3d array representative given a mesh
class SdfModel {
private:
  glm::vec3 cubeSize;
  int cubeCount; // for 1 dimension

public:
  std::optional<Texture3D> texture3D;

  // TODO: move to private, public is for debugging
  std::vector<float> texture3D_data;
  std::vector<std::vector<std::vector<float>>> distances;
  std::vector<std::vector<std::vector<glm::vec3>>>
      positions; // Only used for debugging
  std::vector<std::pair<glm::vec3, glm::vec3>>
      faceNormals; // Only used for debugging

  glm::vec3 facePoint;
  std::vector<std::tuple<glm::vec3, glm::vec3, glm::vec3>>
      isectPoints; // debugs

  BoundingBox outerBB; // bb for sdf
  BoundingBox bb; // mesh bb

  // Will generate SDF on CPU
  SdfModel(Mesh &mesh, int cubeCount = 16) :
      cubeCount(cubeCount),
      outerBB(mesh.boundingBox),
      bb(mesh.boundingBox) {
    // explode the bounding box a little bit
    Transform scaleBB{
        .scale = vec3(1.1, 1.1, 1.1),
    }; // make it a bit bigger
    this->outerBB = this->outerBB.apply_scale(scaleBB);

    cubeSize = vec3((outerBB.max.x - outerBB.min.x) / cubeCount,
                    (outerBB.max.y - outerBB.min.y) / cubeCount,
                    (outerBB.max.z - outerBB.min.z) / cubeCount);
    distances =
        vector(cubeCount, vector(cubeCount, vector(cubeCount, INFINITY)));
    positions = vector(cubeCount, vector(cubeCount, vector(cubeCount, vec3())));

    this->loopOverCubes([&](int k, int j, int i, BoundingBox bb) {
      vector<vec3> isectPoint;
      for (int tri = 0; tri + 2 < mesh.indices.size(); tri += 3) {
        Vertex a = mesh.vertices[mesh.indices[tri]];
        Vertex b = mesh.vertices[mesh.indices[tri + 1]];
        Vertex c = mesh.vertices[mesh.indices[tri + 2]];

        float tIsect;
        if (Util::rayTriangleIntersect(
                bb.center, normalize(vec3(0.0, 1.0, 0.0)), a.position,
                b.position, c.position, tIsect)) {
          vec3 isect = bb.center + vec3(0.0, 1.0, 0.0) * tIsect;
          bool ok = true;
          for (auto ii: isectPoint) {
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
            Util::udTriangle(bb.center, a.position, b.position, c.position);
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
      throw std::runtime_error("unexpected emptiness in sdf distances");
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

  // Skips sdf generation (pass it in through texture3D)
  SdfModel(Mesh &mesh, Texture3D texture3D, int cubeCount) :
      texture3D(std::move(texture3D)),
      cubeCount(cubeCount),
      outerBB(mesh.boundingBox),
      bb(mesh.boundingBox) {
    Transform scaleBB{
        .scale = vec3(1.1, 1.1, 1.1),
    }; // make it a bit bigger
    this->outerBB = this->outerBB.apply_scale(scaleBB);

    cubeSize = vec3((outerBB.max.x - outerBB.min.x) / cubeCount,
                    (outerBB.max.y - outerBB.min.y) / cubeCount,
                    (outerBB.max.z - outerBB.min.z) / cubeCount);

    // flush back all distance to the vector (ergh)
    texture3D_data = this->texture3D->retrieve_data_from_gpu();
  }

  // returns small cubes that creates the sdf
  void loopOverCubes(std::function<void(int, int, int, BoundingBox)> func) {
    // 1. Generate the coordinates for each point that we have inside the
    // bounding box
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

  void bind_to_shader(Shader &shader) {
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


  void writeToFile(std::string path) {
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

  bool find_hit_positions(Ray debugRay, std::vector<glm::vec3> *debugHitPos) {
    // to box only
    // cout << "hit T" << endl;
    // for(int i = 0; i < 20; ++i) {
    //     // first check the box.
    //     float dist = Util::distanceFromBox(debugRay.origin,
    //     this->outerBB.min, this->outerBB.max); debugRay.origin =
    //     debugRay.resolveT(dist); hitPos.push_back(debugRay.origin); cout <<
    //     debugRay.origin.x << " " << debugRay.origin.y << " " <<
    //     debugRay.origin.z << endl;
    // }

    // to box with sdf
    // cout << "hit T" << endl;
    for (int i = 0; i < 100; ++i) {
      if (this->bb.isInside(debugRay.origin)) {
        vec3 localCoord = debugRay.origin - this->outerBB.min;
        vec3 boxArrSize = this->outerBB.getSize() / vec3(this->cubeCount);
        int x = localCoord.x / boxArrSize.x;
        int y = localCoord.y / boxArrSize.y;
        int z = localCoord.z / boxArrSize.z;
        float dist = texture3D_data.at(texture3D->get_index(x, y, z, 1));
        debugRay.origin = debugRay.resolve(dist);

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
        debugRay.origin = debugRay.resolve(dist);
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
};

} // namespace ale::graphics::sdf
