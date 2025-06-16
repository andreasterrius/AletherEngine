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
} // namespace ale::data
