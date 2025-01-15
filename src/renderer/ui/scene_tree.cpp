//
// Created by Alether on 1/13/2025.
//

#include "scene_tree.h"

#include "../../data/scene_node.h"
#include <imgui.h>

std::optional<SceneTree::Entry>
SceneTree::draw_and_handle_clicks(entt::registry &world) {
  ImGui::Begin(panel_name.c_str());

  auto view = world.view<SceneNode>();

  auto entries = std::vector<Entry>();
  for (auto [entity, scene_node] : view.each()) {
    auto name = scene_node.get_name();
    auto id = entt::to_integral(entity);
    entries.emplace_back(id, name);
  }

  std::optional<Entry> clicked = std::nullopt;
  for (auto &entry : entries) {
    if (ImGui::Selectable(("##sn-" + std::to_string(entry.id)).c_str(), false,
                          ImGuiSelectableFlags_SpanAllColumns,
                          ImVec2(0, 100))) {
      clicked = entry;
    }
    ImGui::Text(entry.name.c_str());
    ImGui::Spacing();
  }

  ImGui::End();
  return clicked;
}