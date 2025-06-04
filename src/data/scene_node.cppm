//
// Created by Alether on 1/13/2025.
//
module;
#include <nlohmann/json.hpp>
#include <string>

export module scene_node;

export namespace ale::data {
struct SceneNode {
  std::string name;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SceneNode, name);
} // namespace ale::data
