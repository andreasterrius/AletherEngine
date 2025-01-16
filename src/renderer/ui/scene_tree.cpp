//
// Created by Alether on 1/13/2025.
//

#include "scene_tree.h"

#include "../../data/scene_node.h"
#include <imgui.h>

std::optional<SceneTree::Entry>
SceneTree::draw_and_handle_clicks(entt::registry &world,
                                  std::optional<entt::entity> selected_entity) {
  ImGui::Begin(panel_name.c_str());

  auto view = world.view<SceneNode>();

  auto entries = std::vector<Entry>();
  for (auto [entity, scene_node] : view.each()) {
    auto name = scene_node.get_name();
    auto id = to_integral(entity);
    auto currently_selected =
        selected_entity ? *selected_entity == entity : false;
    entries.emplace_back(id, name, currently_selected);
  }

  std::optional<Entry> clicked = std::nullopt;
  for (auto &entry : entries) {
    // Combine the ID and the name for a unique and meaningful label
    std::string label = entry.name + "##sn-" + std::to_string(entry.id);

    // Use the name directly in the Selectable label
    if (ImGui::Selectable(label.c_str(), entry.currently_selected,
                          ImGuiSelectableFlags_SpanAllColumns, ImVec2(0, 20))) {
      clicked = entry;
    }

    ImGui::Spacing(); // Optional, for visual separation
  }

  ImGui::End();
  return clicked;
}