//
// Created by Alether on 1/13/2025.
//

#ifndef SCENE_TREE_H
#define SCENE_TREE_H
#include <entt/entt.hpp>
#include <imgui.h>
#include <optional>
#include <string>

class SceneTree {
public:
  const std::string panel_name = "Scene Tree";

  struct Entry {
    int id;
    std::string name;
  };

  std::optional<Entry> draw_and_handle_clicks(entt::registry &world);
};

#endif // SCENE_TREE_H
