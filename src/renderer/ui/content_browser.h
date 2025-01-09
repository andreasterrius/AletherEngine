//
// Created by Alether on 1/7/2025.
//

#ifndef CONTENT_BROWSER_H
#define CONTENT_BROWSER_H

#include "../../file_system.h"

#include <ostream>
#include <string>
#include <vector>

using namespace std;
using afs = ale::FileSystem;

namespace ale::ui {
class ContentBrowser {
  string browse_path;
  vector<FileMeta> files;

public:
  ContentBrowser(string browse_path = afs::root("resources"));

  void refresh_files();

  void draw();
};
} // namespace ale::ui

#endif // CONTENT_BROWSER_H
