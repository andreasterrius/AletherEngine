
#ifndef ALETHERENGINE_STATIC_MESH_H
#define ALETHERENGINE_STATIC_MESH_H

#include <memory>

#include "../sdf_model.h"
#include "../sdf_model_packed.h"
#include "model.h"
#include "src/sdf_generator_gpu.h"

using namespace std;

namespace ale {

// This class is safe to be copied.
class StaticMesh {
public:
  struct Meta {
    bool cast_shadow = true;
  };

private:
  shared_ptr<Model> model;
  Meta meta;

  // shadow
  shared_ptr<SdfModelPacked> sdf_model_packed;
  vector<unsigned int> sdf_model_packed_index;

public:
  StaticMesh(shared_ptr<Model> model,
             shared_ptr<SdfModelPacked> sdf_model_packed = nullptr,
             vector<unsigned int> sdf_model_packed_index = {});

  void set_cast_shadow(bool cast_shadow);
  bool get_cast_shadow();

  shared_ptr<Model> get_model();
  pair<shared_ptr<SdfModelPacked>, vector<unsigned int>> get_model_shadow();
};

const string SM_UNIT_CUBE = "default_cube";
const string SM_UNIT_SPHERE = "default_sphere";

class StaticMeshLoader {
  SdfGeneratorGPU sdf_generator_gpu;
  shared_ptr<SdfModelPacked> packed; // OWNING pointer
  unordered_map<string, StaticMesh> static_meshes;

public:
  StaticMeshLoader();

  StaticMesh load_static_mesh(string path);

  StaticMesh create_static_mesh(string id, Model model);

  optional<StaticMesh> get_static_mesh(string id);

  unordered_map<string, StaticMesh> &get_static_meshes();

private:
  optional<Texture3D> load_cached_sdf(int res, const string &sdf_name);

  void save_sdf(Texture3D &sdf, const string &sdf_name);

  std::string hash_sdf_name(std::string sdf_name);
};

}; // namespace ale

#endif