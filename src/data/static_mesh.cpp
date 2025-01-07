#include "static_mesh.h"

#include <utility>

StaticMesh::StaticMesh(shared_ptr<Model> model,
                       shared_ptr<SdfModelPacked> sdf_model_packed,
                       vector<unsigned int> sdf_model_packed_index)
    : model(std::move(model)), sdf_model_packed(std::move(sdf_model_packed)),
      sdf_model_packed_index(std::move(sdf_model_packed_index)) {}

shared_ptr<Model> StaticMesh::get_model() { return model; }

pair<shared_ptr<SdfModelPacked>, vector<unsigned int>>
StaticMesh::get_model_shadow() {
  return make_pair(sdf_model_packed, sdf_model_packed_index);
}

StaticMeshLoader::StaticMeshLoader()
    : packed(make_shared<SdfModelPacked>(vector<SdfModel *>(), false)) {}

StaticMesh StaticMeshLoader::load_static_mesh(string path) {
  const int res = 64;
  auto model = make_shared<Model>(path);
  auto indices = vector<unsigned int>();
  for (int i = 0; i < model->meshes.size(); ++i) {
    string name = path + "_" + to_string(i);
    sdf_generator_gpu.add_mesh(name, model->meshes[i], res, res, res);
    sdf_generator_gpu.generate_all();
    auto texture3d = move(sdf_generator_gpu.at(name));

    auto sdf_model = SdfModel(model->meshes[i], std::move(texture3d), res);
    auto index = packed->add(sdf_model);
    indices.push_back(index);
  }

  return {model, packed, indices};
}
