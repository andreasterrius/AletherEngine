//
// Created by Alether on 1/20/2025.
//
module;

#include <entt/entt.hpp>
#include <fstream>
#include <glm/glm.hpp>
#include <rapidjson/prettywriter.h> // Or just Writer
#include <rapidjson/stringbuffer.h>
#include <spdlog/spdlog.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "rapidjson/document.h"


export module world;
import common;
import scene_node;
import transform;
import light;
import static_mesh;
import file_system;

using namespace ale::data;
using namespace ale::graphics;
using namespace std;
using namespace glm;
using afs = ale::FileSystem;

export namespace ale::serde {

entt::registry load_world(string file_path) {

  rapidjson::Document doc;
  // 1. Open File
  {
    ifstream file(afs::root(file_path));
    std::stringstream ss;
    ss << file.rdbuf();
    doc.Parse(ss.str().c_str());
    file.close();
  }
  if (doc.HasParseError()) {
    throw runtime_error(format("unable to load world {}", file_path));
  }

  // 2. Read and put into registry
  read_vec<vec3>(doc, "test_vec3");
  read_vec<vec4>(doc, "test_vec4");

  entt::registry world;
  return world;
}

void save_world(entt::registry &world, string file_path) {

  // 1. Create json document
  rapidjson::Document doc;
  doc.SetObject();
  auto &allocator = doc.GetAllocator();

  // 2. Fill in json document
  write_vec<vec3>(doc, "test_vec3", vec3(1.0f, 2.0f, 3.0f), allocator);
  write_vec<vec4>(doc, "test_vec4", vec4(1.0f, 2.0f, 3.0f, 4.0f), allocator);

  // 3. Create the string
  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(
      buffer); // PrettyWriter gives nice formatting
  doc.Accept(writer);

  // 3. Write to file
  ofstream file(file_path);
  file << buffer.GetString() << endl;
  file.close();
}


//
// template<typename T>
// optional<T> get(T *ptr) {
//   if (ptr == nullptr)
//     return nullopt;
//   else
//     return *ptr;
// }
//
// template<typename T, typename K>
// K to_serde(T &&item) {
//   return item.to_serde();
// }
//
//
// struct Entity {
//   int entity_id;
//   Transform transform;
//   // std::optional<Light> light;
//   // std::optional<StaticMesh::Serde> static_mesh;
//   // std::optional<SceneNode> scene_node;
// };
// // NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Entity, entity_id, transform, light,
// //                                    static_mesh, scene_node);
// NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Entity, entity_id, transform);
//
// struct Scene {
//   std::vector<Entity> entities;
// };
// NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Scene, entities);
//
// void save_world(string file_path, entt::registry &world) {
//   auto entities = vector<Entity>();
//   auto entities_view = world.view<entt::entity>();
//   for (auto entity: entities_view) {
//     auto [transform, light, static_mesh, scene_node] =
//         world.try_get<Transform, Light, StaticMesh, SceneNode>(entity);
//
//     entities.emplace_back(Entity{
//         .entity_id = static_cast<int>(to_integral(entity)),
//         // .transform = get(transform),
//         // .light = get(light),
//         // .static_mesh =
//         //     get(static_mesh).transform([](auto &&v) { return v.to_serde();
//         //     }),
//         // .scene_node = get(scene_node)
//     });
//   }
//
//   auto scene = Scene{
//       .entities = entities,
//   };
//   nlohmann::json json = scene;
//
//   ofstream file(file_path);
//   file << json << endl;
//   file.close();
//
//   SPDLOG_INFO("saved world to {}", file_path);
// }
//
// // namespace nlohmann {
// // template <> struct adl_serializer<Scene> {
// //   // note: the return type is no longer 'void', and the method only takes
// //   // one argument
// //   static Scene from_json(const ::nlohmann::json &j) {
// //     return {j.at("entities").get<vector<Entity>>()};
// //   }
// //
// //   // Here's the catch! You must provide a to_json method! Otherwise, you
// //   // will not be able to convert move_only_type to json, since you fully
// //   // specialized adl_serializer on that type
// //   static void to_json(::nlohmann::json &j, Scene t) {
// //     j["entities"] = t.entities;
// //   }
// // };
// // } // namespace nlohmann
//
// entt::registry load_world(string file_path, StaticMeshLoader &sm_loader) {
//   ifstream file(afs::root(file_path));
//   auto json = nlohmann::ordered_json::parse(file);
//   file.close();
//
//   auto scene = json.get<Scene>();
//   auto world = entt::registry();
//
//   ranges::reverse(scene.entities);
//
//   // load static meshes first
//   for (auto entity: scene.entities) {
//     auto new_entity = world.create();
//     // if (entity.static_mesh) {
//     //   auto sm =
//     sm_loader.load_static_mesh(entity.static_mesh->model_path);
//     //   sm.set_meta(entity.static_mesh->meta);
//     //   world.emplace<StaticMesh>(new_entity, sm);
//     // }
//     // if (entity.light)
//     //   world.emplace<Light>(new_entity, *entity.light);
//     // if (entity.scene_node)
//     //   world.emplace<SceneNode>(new_entity, *entity.scene_node);
//     // if (entity.transform)
//     //   world.emplace<Transform>(new_entity, *entity.transform);
//   }
//
//   SPDLOG_INFO("loaded world from {}", file_path);
//   return world;
// }

} // namespace ale::serde
