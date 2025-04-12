//
// Created by Alether on 4/12/2025.
//
module;

#include <imgui.h>
#include <spdlog/spdlog.h>
#include <string>
#include <unordered_map>
#include "src/data/file_system.h"
#include "src/graphics/static_mesh.h"
#include "src/graphics/texture.h"
#include "src/graphics/thumbnail_generator.h"

export module content_browser;

import material;

export namespace ale::editor {
class ContentBrowser {
  using afs = FileSystem;

public:
  const std::string panel_name = "Content Browser";

  struct Entry {
    std::string name;
    std::shared_ptr<Texture> thumbnail;

    // if entry contains a static mesh;
    StaticMesh static_mesh;
    BasicMaterial basic_material;

    // if entry is from a file
    std::optional<FileMeta> file_meta;
  };

private:
  std::string browse_path;
  ThumbnailGenerator thumbnail_generator;

public:
  std::unordered_map<std::string, Entry> entries; // temporary for debugging
  ContentBrowser(StaticMeshLoader &sm_loader, string browse_path) :
      browse_path(browse_path) {

    for (auto &[id, sm]: sm_loader.get_static_meshes()) {
      auto model_name = sm.get_model()->path.filename().string();
      entries.emplace(sm.get_model()->path.generic_string(),
                      Entry{.name = model_name,
                            .thumbnail = thumbnail_generator.generate(sm),
                            .static_mesh = sm});
    }

    refresh_files(sm_loader);
  }

  void refresh_files(StaticMeshLoader &sm_loader) {
    SPDLOG_TRACE("Refreshing contents of {}", browse_path);
    auto file_metas = afs::list(browse_path);
    for (auto &file_meta: file_metas) {
      if (entries.find(file_meta.full_path) == entries.end()) {
        // STATIC MESH
        if (file_meta.extension == ".obj" || file_meta.extension == ".gltf") {
          // load static_mesh
          auto [static_mesh, basic_material] =
              sm_loader.load_static_mesh_with_basic_material(
                  file_meta.full_path);

          // generate thumbnail
          auto thumbnail = thumbnail_generator.generate(static_mesh);

          auto entry = Entry{
              .name = file_meta.file_name,
              .thumbnail = thumbnail,
              .static_mesh = static_mesh,
              .basic_material = basic_material,
              .file_meta = file_meta,
          };

          entries.emplace(file_meta.full_path, entry);
        }
      }
    }
    SPDLOG_TRACE("Finish refreshing contents of {}", browse_path);
  }

  optional<Entry> draw_and_handle_clicks() {
    optional<Entry> clicked = nullopt;

    ImGui::Begin(panel_name.c_str(), nullptr, ImGuiWindowFlags_NoCollapse);
    float total_width = ImGui::GetContentRegionAvail().x;
    int each_item_minimum_width = 300;
    int columns = std::max(1, (int) total_width / each_item_minimum_width);
    ImGui::Columns(columns);
    for (auto &[key, entry]: entries) {
      if (ImGui::Selectable(("##cb-" + key).c_str(), false,
                            ImGuiSelectableFlags_None, ImVec2(0, 100))) {
        clicked = entry;
      }
      ImGui::SameLine();
      ImGui::Image(entry.thumbnail->id, ImVec2(100, 100), ImVec2(0, 1),
                   ImVec2(1, 0));
      ImGui::SameLine();
      ImGui::Text(entry.name.c_str());
      ImGui::NextColumn();

      // ImGui::Spacing();
    }
    ImGui::Columns(1);
    ImGui::End();

    return clicked;
  }
};
} // namespace ale::editor
