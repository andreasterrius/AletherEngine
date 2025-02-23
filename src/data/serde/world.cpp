//
// Created by Alether on 1/20/2025.
//

#include "world.h"

#include "src/data/file_system.h"
#include "src/data/scene_node.h"
#include "src/graphics/renderer/basic_renderer.h"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <fstream>

using afs = ale::FileSystem;

namespace ale {
void serde::save_world(string file_path, entt::registry &world) {
  auto entities = vector<Entity>();
  auto entities_view = world.view<entt::entity>();
  for (auto entity : entities_view) {
    auto [transform, light, static_mesh, scene_node] =
        world.try_get<Transform, Light, StaticMesh, SceneNode>(entity);

    entities.emplace_back(
        Entity{.entity_id = static_cast<int>(to_integral(entity)),
               .transform = get(transform),
               .light = get(light),
               .static_mesh = get(static_mesh)->to_serde(),
               .scene_node = get(scene_node)});
  }

  auto scene = Scene{
      .entities = entities,
  };
  nlohmann::json json = scene;

  ofstream file(afs::root(file_path));
  file << json << endl;
  file.close();

  SPDLOG_INFO("saved world to {}", file_path);
}

entt::registry serde::load_world(string file_path,
                                 StaticMeshLoader &sm_loader) {
  ifstream file(afs::root(file_path));
  auto json = nlohmann::ordered_json::parse(file);
  file.close();

  auto scene = json.get<Scene>();
  auto world = entt::registry();

  ranges::reverse(scene.entities);

  // load static meshes first
  for (auto entity : scene.entities) {
    auto new_entity = world.create();
    if (entity.static_mesh) {
      auto sm = sm_loader.load_static_mesh(entity.static_mesh->model_path);
      sm.set_meta(entity.static_mesh->meta);
      world.emplace<StaticMesh>(new_entity, sm);
    }
    if (entity.light)
      world.emplace<Light>(new_entity, *entity.light);
    if (entity.scene_node)
      world.emplace<SceneNode>(new_entity, *entity.scene_node);
    if (entity.transform)
      world.emplace<Transform>(new_entity, *entity.transform);
  }

  SPDLOG_INFO("loaded world from {}", file_path);

  return world;
}
} // namespace ale