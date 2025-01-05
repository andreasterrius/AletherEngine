#include "static_mesh.h"

#include <utility>

ale::StaticMesh::StaticMesh(shared_ptr<Model> model,
                            optional<ShadowEntry> sdf_shadow)
    : model(std::move(model)), sdf_shadow(std::move(sdf_shadow)) {}

shared_ptr<Model> ale::StaticMesh::get_model() { return model; }

optional<ShadowEntry> &ale::StaticMesh::get_sdf_shadow() { return sdf_shadow; }
