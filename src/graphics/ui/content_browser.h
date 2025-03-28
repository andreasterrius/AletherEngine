//
// Created by Alether on 1/7/2025.
//

#ifndef CONTENT_BROWSER_H
#define CONTENT_BROWSER_H

#include "src/data/file_system.h"
#include "src/graphics/thumbnail_generator.h"
#include <string>

using afs = ale::FileSystem;

namespace ale::ui {

// TODO: This class is too big, somehow it also handles loading and stuff.
// But I am not sure how to refactor this right now
class ContentBrowser {

public:
  const string panel_name = "Content Browser";

  struct Entry {
    string name;
    shared_ptr<Texture> thumbnail;

    // if entry contains a static mesh;
    StaticMesh static_mesh;

    // if entry is from a file
    optional<FileMeta> file_meta;
  };

private:
  string browse_path;
  ThumbnailGenerator thumbnail_generator;

public:
  unordered_map<string, Entry> entries; // temporary for debugging
  ContentBrowser(StaticMeshLoader &sm_loader,
                 string browse_path = afs::root("resources"));

  void refresh_files(StaticMeshLoader &sm_loader);

  optional<Entry> draw_and_handle_clicks();
};
} // namespace ale::ui

#endif // CONTENT_BROWSER_H
