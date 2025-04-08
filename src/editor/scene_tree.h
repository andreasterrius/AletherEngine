//
// Created by Alether on 1/13/2025.
//

#ifndef SCENE_TREE_H
#define SCENE_TREE_H

#include <entt/entt.hpp>
#include <optional>
#include <string>

class SceneTree {
public:
  const std::string panel_name = "Scene Tree";

  struct Entry {
    int id;
    std::string name;
    bool currently_selected;
  };

  std::optional<Entry>
  draw_and_handle_clicks(entt::registry &world,
                         std::optional<entt::entity> selected_entity);
};

#endif // SCENE_TREE_H
