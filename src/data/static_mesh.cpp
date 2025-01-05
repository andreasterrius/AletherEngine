#include "static_mesh.h"

#include <utility>

StaticMesh::StaticMesh(shared_ptr<Model> model,
                       shared_ptr<SdfModelPacked> sdf_model_packed,
                       unsigned int sdf_model_packed_index)
    : model(std::move(model)), sdf_model_packed(std::move(sdf_model_packed)),
      sdf_model_packed_index(sdf_model_packed_index) {}

shared_ptr<Model> StaticMesh::get_model() { return model; }

pair<shared_ptr<SdfModelPacked>, unsigned int> StaticMesh::get_model_shadow() {
  return make_pair(sdf_model_packed, sdf_model_packed_index);
}
