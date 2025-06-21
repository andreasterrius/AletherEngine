//
// Created by Alether on 1/20/2025.
//
module;

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <rfl.hpp>
#include <rfl/json.hpp>
#include <string>
#include <vector>

export module serde:world;
import data;
import graphics;
import :common;

using namespace ale::data;
using namespace ale::graphics;
using namespace std;
using namespace glm;

export namespace ale::serde {

struct SavedEntity {
  optional<Transform> transform = nullopt;
  optional<Light> light = nullopt;
  optional<AmbientLight> ambient_light = nullopt;
  optional<SceneNode> scene_node = nullopt;
};

struct SavedRegistry {
  vector<SavedEntity> saved_entities;
};

void save_world(entt::registry &world, string file_path) {
  auto saved_registry = SavedRegistry{};

  // 1. Loop over entities
  // saved_registry.saved_entities.emplace_back(SavedEntity{vec3{1, 2, 3}});
  for (auto &entity: world.view<entt::entity>()) {
    auto saved_entity = SavedEntity{};
    if (auto transform = world.try_get<Transform>(entity)) {
      saved_entity.transform = *transform;
    }
    if (auto light = world.try_get<Light>(entity)) {
      saved_entity.light = *light;
    }
    if (auto ambient_light = world.try_get<AmbientLight>(entity)) {
      saved_entity.ambient_light = *ambient_light;
    }
    if (auto scene_node = world.try_get<SceneNode>(entity)) {
      saved_entity.scene_node = *scene_node;
    }
    saved_registry.saved_entities.emplace_back(saved_entity);
  }

  // 2. Create the string
  const std::string json_string = rfl::json::write(saved_registry);

  // 3. Write to file
  ofstream file(file_path);
  file << json_string << endl;
  file.close();
}

entt::registry load_world(string file_path) {
  // 1. Open File
  {
    ifstream file(afs::root(file_path));
    std::stringstream ss;
    ss << file.rdbuf();
    try {
      auto result = rfl::json::read<SavedRegistry>(ss);
    } catch (exception &e) {
      // TODO: log error out
    }
    file.close();
  }

  return entt::registry{};
}

// entt::registry load_world(string file_path) {
//
//   rapidjson::Document doc;
//   // 1. Open File
//   {
//     ifstream file(afs::root(file_path));
//     std::stringstream ss;
//     ss << file.rdbuf();
//     doc.Parse(ss.str().c_str());
//     file.close();
//   }
//   if (doc.HasParseError()) {
//     throw runtime_error(format("unable to load world {}", file_path));
//   }
//
//   // 2. Read and put into registry
//   read_vec<vec3>(doc, "test_vec3");
//   read_vec<vec4>(doc, "test_vec4");
//
//   entt::registry world;
//   return world;
// }


} // namespace ale::serde
