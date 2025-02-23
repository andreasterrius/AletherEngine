//
// Created by Alether on 1/20/2025.
//

#ifndef WORLD_H
#define WORLD_H

#include "src/data/scene_node.h"
#include "src/data/serde/std.h"
#include "src/graphics/renderer/basic_renderer.h"
#include "src/graphics/static_mesh.h"
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

namespace ale::serde {

struct Entity {
  int entity_id;
  std::optional<Transform> transform;
  std::optional<Light> light;
  std::optional<StaticMesh::Serde> static_mesh;
  std::optional<SceneNode> scene_node;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Entity, entity_id, transform, light,
                                   static_mesh, scene_node);

struct Scene {
  std::vector<Entity> entities;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Scene, entities);

// namespace nlohmann {
// template <> struct adl_serializer<Scene> {
//   // note: the return type is no longer 'void', and the method only takes
//   // one argument
//   static Scene from_json(const ::nlohmann::json &j) {
//     return {j.at("entities").get<vector<Entity>>()};
//   }
//
//   // Here's the catch! You must provide a to_json method! Otherwise, you
//   // will not be able to convert move_only_type to json, since you fully
//   // specialized adl_serializer on that type
//   static void to_json(::nlohmann::json &j, Scene t) {
//     j["entities"] = t.entities;
//   }
// };
// } // namespace nlohmann

template <typename T> optional<T> get(T *ptr) {
  if (ptr == nullptr)
    return nullopt;
  else
    return *ptr;
}
void save_world(string file_path, entt::registry &registry);
entt::registry load_world(string file_path, StaticMeshLoader &sm_loader);

} // namespace ale::serde

#endif // WORLD_H
