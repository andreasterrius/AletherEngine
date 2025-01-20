#include "static_mesh.h"

#include "../file_system.h"
#include <filesystem>
#include <fstream>
#include <spdlog/spdlog.h>
#include <utility>
#include <zstd.h>

namespace fs = std::filesystem;
using afs = ale::FileSystem;

StaticMesh::StaticMesh(shared_ptr<Model> model,
                       shared_ptr<SdfModelPacked> sdf_model_packed,
                       vector<unsigned int> sdf_model_packed_index)
    : model(std::move(model)), sdf_model_packed(std::move(sdf_model_packed)),
      sdf_model_packed_index(std::move(sdf_model_packed_index)) {}

void StaticMesh::set_cast_shadow(bool cast_shadow) {
  this->meta.cast_shadow = cast_shadow;
}

bool StaticMesh::get_cast_shadow() { return this->meta.cast_shadow; }

void StaticMesh::set_meta(Meta meta) { this->meta = meta; }

shared_ptr<Model> StaticMesh::get_model() { return model; }

pair<shared_ptr<SdfModelPacked>, vector<unsigned int>>
StaticMesh::get_model_shadow() {
  return make_pair(sdf_model_packed, sdf_model_packed_index);
}

StaticMesh::Serde StaticMesh::to_serde() {
  return Serde{
      .meta = meta,
      .model_path = model->path.string(),
  };
}

StaticMeshLoader::StaticMeshLoader()
    : packed(make_shared<SdfModelPacked>(vector<SdfModel *>(), false)) {
  this->load_static_mesh(afs::root("resources/default_models/unit_cube.obj"),
                         {SM_UNIT_CUBE});
  this->load_static_mesh(afs::root("resources/default_models/unit_sphere.obj"),
                         {SM_UNIT_SPHERE});
}

StaticMesh StaticMeshLoader::load_static_mesh(string path) {
  return load_static_mesh(path, {});
}

StaticMesh StaticMeshLoader::load_static_mesh(string path,
                                              vector<string> alternate_names) {
  string id = afs::from_root(path);
  auto it = this->static_meshes.find(id);
  if (it != this->static_meshes.end()) {
    return it->second;
  }
  auto start_time = std::chrono::high_resolution_clock::now();
  const int res = 64;

  auto model = Model(path);
  auto indices = vector<unsigned int>();
  for (int i = 0; i < model.meshes.size(); ++i) {
    string name = id + "_" + to_string(i);

    auto cached = load_cached_sdf(res, name);
    if (cached) {
      auto sdf_model = SdfModel(model.meshes[i], std::move(*cached), res);
      auto index = packed->add(sdf_model);
      indices.push_back(index);
    } else {
      sdf_generator_gpu.add_mesh(name, model.meshes[i], res, res, res);
      sdf_generator_gpu.generate_all();

      auto sdf_model =
          SdfModel(model.meshes[i], move(sdf_generator_gpu.at(name)), res);
      auto index = packed->add(sdf_model);
      indices.push_back(index);

      this->save_sdf(*sdf_model.texture3D, name);
    }
  }

  auto elapsed = std::chrono::high_resolution_clock::now() - start_time;
  SPDLOG_INFO(
      "loaded {}, took {}ms", id,
      std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());

  auto static_mesh =
      StaticMesh{make_shared<Model>(std::move(model)), packed, indices};
  this->static_meshes.emplace(id, static_mesh);

  for (auto &name : alternate_names) {
    this->alternate_names[name] = id;
  }

  return static_mesh;
}

optional<StaticMesh> StaticMeshLoader::get_static_mesh(string id) {
  auto name_it = this->alternate_names.find(id);
  if (name_it != this->alternate_names.end()) {
    id = name_it->second;
  }

  auto it = this->static_meshes.find(id);
  if (it != this->static_meshes.end()) {
    return it->second;
  }
  return nullopt;
}

unordered_map<string, StaticMesh> &StaticMeshLoader::get_static_meshes() {
  return this->static_meshes;
}

optional<Texture3D> StaticMeshLoader::load_cached_sdf(int res,
                                                      const string &sdf_name) {

  string sdf_cache_name = this->hash_sdf_name(sdf_name);
  string sdf_cache_path =
      afs::root("resources/sdf_cache/" + sdf_cache_name + ".bin");

  // let's escape the name since it's possibe that it's a path
  auto texture_file = ifstream{}; // open files
  texture_file.exceptions(std::ifstream::badbit);
  texture_file.open(sdf_cache_path, std::ios::binary);
  if (!texture_file.is_open()) {
    return nullopt;
  }

  SPDLOG_TRACE("loading {} -> {}", sdf_cache_path, sdf_name);

  texture_file.seekg(0, std::ios::end);
  std::streamsize filesize = texture_file.tellg();
  texture_file.seekg(0, std::ios::beg);

  size_t num_floats = filesize / sizeof(float);

  vector<float> texture_data(num_floats);
  if (texture_file.read(reinterpret_cast<char *>(texture_data.data()),
                        filesize)) {
    return Texture3D(Texture3D::Meta{.width = res,
                                     .height = res,
                                     .depth = res,
                                     .internal_format = GL_R32F,
                                     .input_format = GL_RED,
                                     .input_type = GL_FLOAT},
                     texture_data);
  }

  return nullopt;
}

void StaticMeshLoader::save_sdf(Texture3D &sdf, const string &sdf_name) {

  string sdf_cache_name = this->hash_sdf_name(sdf_name);
  string sdf_cache_path =
      afs::root("resources/sdf_cache/" + sdf_cache_name + ".bin");

  SPDLOG_TRACE("saving sdf {} -> {}", sdf_name, sdf_cache_path);

  auto data = sdf.retrieve_data_from_gpu();
  ofstream out_file(sdf_cache_path, std::ios::binary);
  out_file.write(reinterpret_cast<const char *>(data.data()),
                 data.size() * sizeof(float));
  out_file.close();
}

std::string StaticMeshLoader::hash_sdf_name(std::string sdf_name) {
  std::hash<std::string> hasher;
  size_t hashedValue = hasher(sdf_name);
  return to_string(hashedValue);
}
