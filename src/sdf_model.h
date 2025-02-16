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
#include "data/mesh.h"
#include "data/ray.h"
#include "data/shader.h"
#include "texture.h"

namespace ale {

class Model;
class LineRenderer;

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
  BoundingBox bb;      // mesh bb

  // Will generate SDF on CPU
  SdfModel(Mesh &mesh, int cubeCount = 16);

  // Skips sdf generation (pass it in through texture3D)
  SdfModel(Mesh &mesh, Texture3D texture3D, int cubeCount);

  // returns small cubes that creates the sdf
  void loopOverCubes(std::function<void(int, int, int, BoundingBox)> func);

  void bind_to_shader(Shader shader);

  void writeToFile(std::string path);

  bool find_hit_positions(Ray debugRay, std::vector<glm::vec3> *debugHitPos);
};

} // namespace ale

#endif // ALETHERENGINE_SDF_MODEL_H
