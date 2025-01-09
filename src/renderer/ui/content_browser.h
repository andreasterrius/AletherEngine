//
// Created by Alether on 1/7/2025.
//

#ifndef CONTENT_BROWSER_H
#define CONTENT_BROWSER_H

#include "../../file_system.h"
#include "../thumbnail_generator.h"

#include <string>

using namespace std;
using afs = ale::FileSystem;

namespace ale::ui {

// TODO: This class is too big, somehow it also handles loading and stuff.
// But I am not sure how to refactor this right now
class ContentBrowser {

  struct ContentBrowserEntry {
    FileMeta file_meta;
    shared_ptr<Texture> thumbnail;
  };

  string browse_path;
  unordered_map<string, ContentBrowserEntry> entries;
  ThumbnailGenerator thumbnail_generator;
  StaticMeshLoader static_mesh_loader;

public:
  ContentBrowser(string browse_path = afs::root("resources"));

  void refresh_files();

  void draw();
};
} // namespace ale::ui

#endif // CONTENT_BROWSER_H
