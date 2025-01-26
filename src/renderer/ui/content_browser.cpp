//
// Created by Alether on 1/7/2025.
//

#include "content_browser.h"
#include "../thumbnail_generator.h"

#include <imgui.h>
#include <spdlog/spdlog.h>
#include <vector>

using namespace ale;

ui::ContentBrowser::ContentBrowser(StaticMeshLoader &sm_loader,
                                   string browse_path)
    : browse_path(browse_path) {

  for (auto &[id, sm] : sm_loader.get_static_meshes()) {
    auto model_name = sm.get_model()->path.filename().string();
    entries.emplace(sm.get_model()->path.generic_string(),
                    Entry{.name = model_name,
                          .thumbnail = thumbnail_generator.generate(sm),
                          .static_mesh = sm});
  }

  refresh_files(sm_loader);
}

void ui::ContentBrowser::refresh_files(StaticMeshLoader &sm_loader) {
  SPDLOG_TRACE("Refreshing contents of {}", browse_path);
  auto file_metas = afs::list(browse_path);
  for (auto &file_meta : file_metas) {
    if (entries.find(file_meta.full_path) == entries.end()) {
      // STATIC MESH
      if (file_meta.extension == ".obj") {
        // load static_mesh
        auto static_mesh = sm_loader.load_static_mesh(file_meta.full_path);

        // generate thumbnail
        auto thumbnail = thumbnail_generator.generate(static_mesh);

        auto entry = Entry{
            .name = file_meta.file_name,
            .thumbnail = thumbnail,
            .static_mesh = static_mesh,
            .file_meta = file_meta,
        };

        entries.emplace(file_meta.full_path, entry);
      }
    }
  }
  SPDLOG_TRACE("Finish refreshing contents of {}", browse_path);
}

optional<ui::ContentBrowser::Entry>
ui::ContentBrowser::draw_and_handle_clicks() {
  optional<Entry> clicked = nullopt;

  ImGui::Begin(panel_name.c_str(), nullptr, ImGuiWindowFlags_NoCollapse);
  float total_width = ImGui::GetContentRegionAvail().x;
  int each_item_minimum_width = 300;
  int columns = std::max(1, (int)total_width / each_item_minimum_width);
  ImGui::Columns(columns);
  for (auto &[key, entry] : entries) {
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