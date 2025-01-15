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
  refresh_files(sm_loader);
}

void ui::ContentBrowser::refresh_files(StaticMeshLoader &sm_loader) {
  SPDLOG_DEBUG("Refreshing contents of {}", browse_path);

  auto file_metas = afs::list(browse_path);
  for (auto &file_meta : file_metas) {
    if (entries.find(file_meta.full_path) == entries.end()) {
      // STATIC MESH
      if (file_meta.extension == ".obj") {
        // load static_mesh
        auto static_mesh = sm_loader.load_static_mesh(file_meta.full_path);

        // generate thumbnail
        auto thumbnail = thumbnail_generator.generate(static_mesh);

        auto entry = Entry{};
        entry.file_meta = file_meta;
        entry.thumbnail = thumbnail;
        entry.static_mesh = static_mesh;

        entries.emplace(file_meta.full_path, entry);
      }
    }
  }
  SPDLOG_DEBUG("Finish refreshing contents of {}", browse_path);
}

optional<ui::ContentBrowser::Entry>
ui::ContentBrowser::draw_and_handle_clicks() {
  optional<Entry> clicked = nullopt;

  ImGui::Begin(panel_name.c_str());
  for (auto &[key, entry] : entries) {
    if (ImGui::Selectable(("##cb-" + key).c_str(), false,
                          ImGuiSelectableFlags_SpanAllColumns,
                          ImVec2(0, 100))) {
      clicked = entry;
    }
    ImGui::SameLine();
    ImGui::Image(entry.thumbnail->id, ImVec2(100, 100), ImVec2(0, 1),
                 ImVec2(1, 0));
    ImGui::SameLine();
    ImGui::Text(entry.file_meta.file_name.c_str());

    ImGui::Spacing();
  }
  ImGui::End();

  return clicked;
}