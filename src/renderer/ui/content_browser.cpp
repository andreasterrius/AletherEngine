//
// Created by Alether on 1/7/2025.
//

#include "content_browser.h"

#include "src/renderer/thumbnail_generator.h"

#include <imgui.h>
#include <vector>

using namespace ale;

ui::ContentBrowser::ContentBrowser(string browse_path)
    : browse_path(browse_path) {
  refresh_files();
}

void ui::ContentBrowser::refresh_files() {
  auto file_metas = afs::list(browse_path);
  for (auto &file_meta : file_metas) {
    if (entries.find(file_meta.full_path) == entries.end()) {
      // STATIC MESH
      if (file_meta.extension == ".obj") {
        // load static_mesh
        auto static_mesh =
            static_mesh_loader.load_static_mesh(file_meta.full_path);

        // generate thumbnail
        auto thumbnail = thumbnail_generator.generate(static_mesh);

        entries.emplace(file_meta.full_path,
                        ContentBrowserEntry{file_meta, thumbnail});
      }
    }
  }
}

void ale::ui::ContentBrowser::draw() {
  // TODO: for now we put the logic to get the stuff here.

  ImGui::Begin("ContentBrowser");
  for (auto &[key, entry] : entries) {
    ImGui::Image(entry.thumbnail->id, ImVec2(100, 100), ImVec2(0, 1),
                 ImVec2(1, 0));
    ImGui::Text(entry.file_meta.file_name.c_str());
  }
  ImGui::End();
}