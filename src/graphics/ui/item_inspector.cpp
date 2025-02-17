//
// Created by Alether on 1/22/2025.
//

#include "item_inspector.h"

#include "src/data/scene_node.h"
#include "src/data/transform.h"
#include <imgui.h>

namespace ale::ui {
ItemInspector::ItemInspector() {}

void ItemInspector::draw_and_handle(entt::registry &world,
                                    std::optional<entt::entity> entity) {

  ImGui::Begin(panel_name.c_str());
  if (entity.has_value()) {
    auto [scene_node, transform] = world.try_get<SceneNode, Transform>(*entity);
    if (create_section("Scene Node", scene_node)) {
      inspect_text("Name", scene_node->name);
    }
    if (create_section("Transform", transform)) {
      inspect_vec3f("Pos", transform->translation);
      inspect_vec3f("Scale", transform->scale);
      inspect_quat("Rot", transform->rotation);
    }
  }

  ImGui::End();
}

bool ItemInspector::create_section(std::string name, void *ptr) {
  if (ptr != nullptr) {
    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    if (ImGui::CollapsingHeader(name.c_str())) {
      return true;
    }
  }
  return false;
}
void ItemInspector::separate_two(std::string name, std::function<void()> func) {
  ImGui::Columns(2, std::format("##{}", name).c_str(), false);

  ImGui::SetColumnWidth(0, 75.0f);
  ImGui::Text(name.c_str());

  ImGui::NextColumn();
  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
  func();

  ImGui::Columns(1);
}

void ItemInspector::inspect_vec3f(std::string name, glm::vec3 &v) {
  separate_two(name, [&]() {
    ImGui::InputFloat3(std::format("##{}", name).c_str(), &v[0]);
  });
}
void ItemInspector::inspect_quat(std::string name, glm::quat quat) {
  separate_two(name, [&]() {
    ImGui::InputFloat4(std::format("##{}", name).c_str(), &quat[0]);
  });
}
void ItemInspector::inspect_text(std::string name, std::string &val) {
  separate_two(name, [&]() {
    ImGui::InputText(std::format("##{}", name).c_str(), val.data(),
                     val.capacity() + 1, ImGuiInputTextFlags_CallbackResize,
                     inspect_text_string_resize, &val);
  });
}

static int inspect_text_string_resize(ImGuiInputTextCallbackData *data) {
  if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
    std::string *str = (std::string *)data->UserData;
    str->resize(data->BufTextLen); // Resize to match new text length
    data->Buf = str->data();       // Update buffer pointer
  }
  return 0;
}
} // namespace ale::ui