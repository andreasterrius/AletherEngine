module;

#include <entt/entt.hpp>
#include <filesystem>
#include <imgui.h>
#include <memory>
#include <optional>
#include <spdlog/spdlog.h>
#include <string>
#include <unordered_map>

export module content_browser;
import texture;
import material;
import file_system;
import default_resources;
import thumbnail_generator;
import static_mesh;
import stash;

using namespace ale::graphics;
using namespace std;

export namespace ale::editor {
class ContentBrowser {
  using afs = FileSystem;

public:
  const std::string panel_name = "Content Browser";

  struct Entry {
    std::string name;
    std::shared_ptr<Texture> thumbnail;

    // if entry contains a static mesh;
    std::optional<pair<StaticMesh, BasicMaterial>> static_mesh_with_material;

    // if entry is from a file
    std::optional<FileMeta> file_meta;
  };

private:
  shared_ptr<Stash<Texture>> texture_stash;
  std::string browse_root_path;
  ThumbnailGenerator thumbnail_generator;

  std::string current_path;
  std::unordered_map<std::string, Entry> entries;

public:
  ContentBrowser(StaticMeshLoader &sm_loader,
                 shared_ptr<Stash<Texture>> texture_stash,
                 string browse_root_path) :
      browse_root_path(browse_root_path),
      texture_stash(texture_stash),
      current_path(browse_root_path) {

    for (auto &[id, sm]: sm_loader.get_static_meshes()) {
      auto model_name = sm.get_model()->path.filename().string();
      entries.emplace(
          sm.get_model()->path.generic_string(),
          Entry{.name = model_name,
                .thumbnail = thumbnail_generator.generate(sm),
                .static_mesh_with_material = make_pair(sm, BasicMaterial{})});
    }

    load_entries(current_path, sm_loader);
  }

  void load_entries(std::string path, StaticMeshLoader &sm_loader) {

    entries.clear();

    SPDLOG_TRACE("Showing entries from {}", path);
    auto file_metas = afs::list(path);
    for (auto &file_meta: file_metas.childs) {
      if (entries.find(file_meta.full_path) == entries.end()) {
        // STATIC MESH
        if (file_meta.extension == ".obj" || file_meta.extension == ".gltf") {
          // load static_mesh
          auto [static_mesh, basic_material] =
              sm_loader.load_static_mesh_with_basic_material(
                  file_meta.full_path);

          // generate thumbnail
          auto thumbnail = texture_stash->get_or(
              file_meta.full_path + "_thumbnail",
              [&](std::string &path) -> shared_ptr<Texture> {
                return thumbnail_generator.generate(static_mesh);
              });

          entries.emplace(file_meta.full_path,
                          Entry{
                              .name = file_meta.file_name,
                              .thumbnail = thumbnail,
                              .static_mesh_with_material =
                                  make_pair(static_mesh, basic_material),
                              .file_meta = file_meta,
                          });
        } else if (file_meta.is_folder) {
          auto thumbnail = texture_stash->get_or(
              afs::root(editor::FOLDER_ICON),
              [&](std::string &path) { return make_shared<Texture>(path); });
          entries.emplace(file_meta.full_path,
                          Entry{
                              .name = file_meta.file_name,
                              .thumbnail = thumbnail,
                              .static_mesh_with_material = nullopt,
                              .file_meta = file_meta,
                          });
        }
      }
    }
  }

  optional<Entry> draw_and_handle_clicks(StaticMeshLoader &sm_loader) {
    optional<Entry> clicked = nullopt;

    ImGui::Begin(panel_name.c_str(), nullptr, ImGuiWindowFlags_NoCollapse);

    ImGui::Text(format("path: {}", current_path).c_str());
    ImGui::Spacing();
    // if (ImGui::Button("Refresh")) {
    //   load_entries(sm_loader);
    // }
    ImGui::Spacing();

    float total_width = ImGui::GetContentRegionAvail().x;
    int each_item_minimum_width = 300;
    int columns = std::max(1, (int) total_width / each_item_minimum_width);
    ImGui::Columns(columns);
    for (auto &[key, entry]: entries) {
      if (ImGui::Selectable(("##cb-" + key).c_str(), false,
                            ImGuiSelectableFlags_None, ImVec2(0, 100))) {
        if (entry.file_meta->is_folder) {
          current_path = entry.file_meta->full_path;
        } else {
          clicked = entry;
        }
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

    load_entries(current_path, sm_loader);

    return clicked;
  }
};
} // namespace ale::editor
