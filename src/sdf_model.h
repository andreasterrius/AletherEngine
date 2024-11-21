//
// Created by Alether on 5/4/2024.
//

#ifndef ALETHERENGINE_SDF_MODEL_H
#define ALETHERENGINE_SDF_MODEL_H

#include <functional>
#include <glm/glm.hpp>
#include <optional>
#include <utility>
#include <vector>

#include "data/boundingbox.h"
#include "data/ray.h"
#include "data/shader.h"
#include "texture.h"

using namespace glm;
using namespace std;

namespace ale {

class Model;
class LineRenderer;

// This is a 3d array representative given a mesh
class SdfModel {
 private:
  vec3 cubeSize;
  int cubeCount;  // for 1 dimension
  bool shouldFlatten;

 public:
  optional<Texture3D> texture3D;

  // TODO: move to private, public is for debugging
  vector<vector<vector<float>>> distances;
  vector<vector<vector<vec3>>> positions;  // Only used for debugging
  vector<pair<vec3, vec3>> faceNormals;    // Only used for debugging

  vec3 facePoint;
  vector<tuple<vec3, vec3, vec3>> isectPoints;  // debugs

  BoundingBox outerBB;  // bb for sdf
  BoundingBox bb;       // mesh bb

  // Will generate SDF on CPU
  SdfModel(Model &model, int cubeCount = 16);

  // Skips sdf generation (pass it in through texture3D)
  SdfModel(Model &model, Texture3D texture3D, int cubeCount);

  // returns small cubes that creates the sdf
  void loopOverCubes(function<void(int, int, int, BoundingBox)> func);

  void bindToShader(Shader shader);

  void writeToFile(string path);

  bool findHitPositions(Ray debugRay, vector<vec3> *debugHitPos);
};

}  // namespace ale

#endif  // ALETHERENGINE_SDF_MODEL_H
