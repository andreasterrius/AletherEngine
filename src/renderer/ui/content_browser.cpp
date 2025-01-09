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

void ui::ContentBrowser::refresh_files() { files = afs::list(browse_path); }

void ale::ui::ContentBrowser::draw() {
  // TODO: for now we put the logic to get the stuff here.

  ImGui::Begin("ContentBrowser");
  for (auto &file : files) {
    ImGui::Text(file.file_name.c_str());
  }
  ImGui::End();
}