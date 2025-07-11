//
// Created by Alether on 1/13/2025.
//

module;
#include <entt/entt.hpp>
#include <imgui.h>
#include <optional>
#include <string>

export module editor:scene_tree;

import data;

using namespace ale::data;

export namespace ale::editor {
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
                         std::optional<entt::entity> selected_entity) {
    ImGui::Begin(panel_name.c_str(), nullptr, ImGuiWindowFlags_NoCollapse);

    auto view = world.view<SceneNode>();

    auto entries = std::vector<Entry>();
    for (auto [entity, scene_node]: view.each()) {
      auto name = scene_node.name;
      auto id = to_integral(entity);
      auto currently_selected =
          selected_entity ? *selected_entity == entity : false;
      entries.emplace_back(id, name, currently_selected);
    }

    std::optional<Entry> clicked = std::nullopt;
    for (auto &entry: entries) {
      // Combine the ID and the name for a unique and meaningful label
      std::string label = entry.name + "##sn-" + std::to_string(entry.id);

      // Use the name directly in the Selectable label
      if (ImGui::Selectable(label.c_str(), entry.currently_selected,
                            ImGuiSelectableFlags_SpanAllColumns,
                            ImVec2(0, 20))) {
        clicked = entry;
      }

      ImGui::Spacing(); // Optional, for visual separation
    }

    ImGui::End();
    return clicked;
  }
};
} // namespace ale::editor
