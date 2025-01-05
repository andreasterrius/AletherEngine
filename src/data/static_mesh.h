
#ifndef ALETHERENGINE_STATIC_MESH_H
#define ALETHERENGINE_STATIC_MESH_H

#include <memory>

#include "../sdf_model.h"
#include "../sdf_model_packed.h"
#include "model.h"
#include "src/sdf_generator_gpu.h"

using namespace std;

namespace ale {

class StaticMesh {
private:
  shared_ptr<Model> model;

  // shadow
  shared_ptr<SdfModelPacked> sdf_model_packed;
  unsigned int sdf_model_packed_index;

public:
  StaticMesh(shared_ptr<Model> model,
             shared_ptr<SdfModelPacked> sdf_model_packed = nullptr,
             unsigned int sdf_model_packed_index = 0);

  shared_ptr<Model> get_model();
  pair<shared_ptr<SdfModelPacked>, unsigned int> get_model_shadow();
};

// class StaticMeshLoader {
//   SDFGeneratorGPU sdf_generator_gpu;
//
// public:
//   StaticMesh load_static_mesh(string path);
// };

}; // namespace ale

#endif