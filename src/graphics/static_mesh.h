
#ifndef ALETHERENGINE_STATIC_MESH_H
#define ALETHERENGINE_STATIC_MESH_H

#include <memory>
#include "nlohmann/json.hpp"
#include "src/graphics/model.h"
#include "src/graphics/sdf/sdf_generator_gpu.h"
#include "src/graphics/sdf/sdf_generator_gpu_v2.h"
#include "src/graphics/sdf/sdf_model.h"
#include "src/graphics/sdf/sdf_model_packed.h"

import material;
import stash;

namespace ale {

// This class is safe to be copied.
class StaticMesh {
public:
  struct Meta {
    bool cast_shadow = true;
  };

  struct Serde {
    Meta meta;
    string model_path;
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

  // only use for loading world
  void set_meta(Meta meta);

  shared_ptr<Model> get_model();
  pair<shared_ptr<SdfModelPacked>, vector<unsigned int>> get_model_shadow();

  Serde to_serde();
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(StaticMesh::Meta, cast_shadow)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(StaticMesh::Serde, meta, model_path)

// alternate names
const string SM_UNIT_CUBE = "default_cube";
const string SM_UNIT_SPHERE = "default_sphere";

// not Send/Sync
class StaticMeshLoader {
  // SdfGeneratorGPU sdf_generator_gpu;
  SdfGeneratorGPUV2 sdf_generator_gpu_v2;
  shared_ptr<SdfModelPacked> packed; // OWNING pointer
  unordered_map<string, StaticMesh> static_meshes;

  // refer to static meshes keys
  unordered_map<string, string> alternate_names;

  // stashes
  shared_ptr<Stash<Texture>> texture_stash;

public:
  StaticMeshLoader(const shared_ptr<Stash<Texture>> &texture_stash);

  StaticMesh load_static_mesh(string path);

  StaticMesh load_static_mesh(string path, vector<string> alternate_names);

  pair<StaticMesh, BasicMaterial>
  load_static_mesh_with_basic_material(string path);

  optional<StaticMesh> get_static_mesh(string id);

  unordered_map<string, StaticMesh> &get_static_meshes();

private:
  optional<Texture3D> load_cached_sdf(int res, const string &sdf_name);

  void save_sdf(Texture3D &sdf, const string &sdf_name);

  std::string hash_sdf_name(std::string sdf_name);
};

}; // namespace ale

#endif
