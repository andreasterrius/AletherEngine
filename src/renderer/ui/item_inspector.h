//
// Created by Alether on 1/22/2025.
//

#ifndef ITEM_INSPECTOR_H
#define ITEM_INSPECTOR_H
#include <entt/entity/registry.hpp>
#include <glm/glm.hpp>
#include <optional>
#include <string>

class ImGuiInputTextCallbackData;

namespace ale::ui {
class ItemInspector {
public:
  std::string panel_name = "Item Inspector";

  ItemInspector();

  void draw_and_handle(entt::registry &world,
                       std::optional<entt::entity> entity);

private:
  bool create_section(std::string name, void *ptr);

  void separate_two(std::string name, std::function<void()> func);
  void inspect_vec3f(std::string name, glm::vec3 &v);
  // void inspect_vec4(std::string name, glm::vec4 &v);
  void inspect_quat(std::string name, glm::quat quat);
  void inspect_text(std::string name, std::string &val);
};

static int inspect_text_string_resize(ImGuiInputTextCallbackData *data);

} // namespace ale::ui

#endif // ITEM_INSPECTOR_H
