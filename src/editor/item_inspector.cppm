//
// Created by Alether on 1/22/2025.
//
module;

#include <entt/entity/registry.hpp>
#include <format>
#include <glm/glm.hpp>
#include <imgui.h>
#include <iostream>
#include <nfd.hpp>
#include <optional>
#include <string>
#include <variant>

export module item_inspector;

import material;
import transform;
import scene_node;
import texture;

using namespace ale::data;
using namespace ale::graphics;

int inspect_text_string_resize(ImGuiInputTextCallbackData *data) {
  if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
    std::string *str = (std::string *) data->UserData;
    str->resize(data->BufTextLen); // Resize to match new text length
    data->Buf = str->data(); // Update buffer pointer
  }
  return 0;
}

export namespace ale::editor {

const std::string DIFFUSE = "Diffuse";
const std::string SPECULAR = "Specular";


class ItemInspector {

public:
  struct LoadTextureCmd {
    std::string type;
    std::string path;
    entt::entity entity_to_load;
  };

  using Cmd = std::variant<LoadTextureCmd>;

public:
  std::string panel_name = "Item Inspector";

  // ItemInspector() {};

  std::vector<Cmd> draw_and_handle(entt::registry &world,
                                   std::optional<entt::entity> entity) {

    auto cmds = std::vector<Cmd>();
    ImGui::Begin(panel_name.c_str());
    if (entity.has_value()) {
      auto [scene_node, transform, basic_material] =
          world.try_get<SceneNode, Transform, BasicMaterial>(*entity);
      if (create_section("Scene Node", scene_node)) {
        inspect_text("Name", scene_node->name);
      }
      if (create_section("Transform", transform)) {
        inspect_vec3f("Pos", transform->translation);
        inspect_vec3f("Scale", transform->scale);
        inspect_quat("Rot", transform->rotation);
      }
      if (create_section("Basic Material", basic_material)) {
        inspect_vec3f("Diffuse Color", basic_material->diffuse_color);

        if (auto load_diffuse = inspect_image(
                DIFFUSE, basic_material->diffuse_texture, *entity)) {
          cmds.emplace_back(*load_diffuse);
        }
        if (auto load_specular = inspect_image(
                SPECULAR, basic_material->specular_texture, *entity)) {
          cmds.emplace_back(*load_specular);
        }
      }
    }
    ImGui::End();

    return cmds;
  }

private:
  bool create_section(std::string name, void *ptr) {
    if (ptr != nullptr) {
      ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
      if (ImGui::CollapsingHeader(name.c_str())) {
        return true;
      }
    }
    return false;
  }

  void separate_two(std::string name, std::function<void()> func) {
    ImGui::Columns(2, std::format("##{}", name).c_str(), false);

    ImGui::SetColumnWidth(0, 75.0f);
    ImGui::Text(name.c_str());

    ImGui::NextColumn();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    func();

    ImGui::Columns(1);
  }

  void inspect_vec3f(std::string name, glm::vec3 &v) {
    separate_two(name, [&]() {
      ImGui::InputFloat3(std::format("##{}", name).c_str(), &v[0]);
    });
  }

  // void inspect_vec4(std::string name, glm::vec4 &v);
  void inspect_quat(std::string name, glm::quat quat) {
    separate_two(name, [&]() {
      ImGui::InputFloat4(std::format("##{}", name).c_str(), &quat[0]);
    });
  }

  void inspect_text(std::string name, std::string &val) {
    separate_two(name, [&]() {
      ImGui::InputText(std::format("##{}", name).c_str(), val.data(),
                       val.capacity() + 1, ImGuiInputTextFlags_CallbackResize,
                       inspect_text_string_resize, &val);
    });
  }

  std::optional<LoadTextureCmd> inspect_image(std::string name,
                                              std::shared_ptr<Texture> texture,
                                              entt::entity entity) {
    float width = 160;
    float height = 160;
    std::optional<LoadTextureCmd> load_event = std::nullopt;
    separate_two(name, [&]() {
      if (texture != nullptr) {
        ImGui::Image(texture.get()->id, ImVec2(width, height));
      } else {
        ImGui::Text("No Texture Loaded");
        ImGui::Dummy(ImVec2(width, height)); // Reserve space
      }
      if (ImGui::IsItemClicked()) {
        NFD::UniquePath out_path;
        nfdresult_t result = NFD::OpenDialog(out_path, nullptr);

        if (result == NFD_OKAY) {
          std::cout << "Selected file: " << out_path.get() << std::endl;
          load_event =
              std::make_optional<LoadTextureCmd>(name, out_path.get(), entity);
        }
      }
    });

    return load_event;
  }
};

} // namespace ale::editor
