//
// Created by Alether on 4/16/2024.
//

#include "file_system.h"
#include <filesystem>

namespace fs = std::filesystem;

namespace ale {
string FileSystem::root(const string &path) {
  return string(ALE_ROOT_PATH) + "/" + path;
}

string FileSystem::from_root(const string &path) {
  fs::path p(path);
  fs::path this_root = root("");

  auto res = fs::relative(p, this_root);
  return res.generic_string();
}

vector<FileMeta> FileSystem::list(const string &path) {
  vector<FileMeta> file_metas;

  // Check if the folder exists
  if (!fs::exists(path) || !fs::is_directory(path)) {
    return {};
  }

  // Iterate through the directory
  for (const auto &entry : fs::directory_iterator(path)) {
    if (entry.is_regular_file()) { // Only include regular files
      file_metas.push_back(
          FileMeta{.full_path = entry.path().string(),
                   .file_name = entry.path().filename().string(),
                   .extension = entry.path().extension().string(),
                   .size = static_cast<unsigned int>(entry.file_size())});
    } else if (entry.is_directory()) {
      auto childs = list(entry.path().string());
      file_metas.insert(file_metas.end(), childs.begin(), childs.end());
    }
  }

  return file_metas;
}

} // namespace ale
