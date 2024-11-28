#include "static_mesh.h"

ale::StaticMesh::StaticMesh(std::shared_ptr<Model> model,
                            std::shared_ptr<SdfModel> sdf_model,
                            std::shared_ptr<SdfModelPacked> sdf_model_packed)
    : model(model), sdf_model(sdf_model), sdf_model_packed(sdf_model_packed) {}

std::shared_ptr<Model> &ale::StaticMesh::get_model() { return model; }
