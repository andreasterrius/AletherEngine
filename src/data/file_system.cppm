module;

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include "src/config.h"

export module data:file_system;

using namespace std;

export namespace ale::data::afs {
namespace fs = std::filesystem;
struct FileMeta {
  std::string full_path;
  std::string file_name;
  std::string extension;
  unsigned int size;
  bool is_folder;

  vector<FileMeta> childs;
};
string root(const string &path) { return string(ALE_ROOT_PATH) + "/" + path; }
string from_root(const string &path) {
  fs::path p(path);
  fs::path this_root = root("");

  auto res = fs::relative(p, this_root);
  return res.generic_string();
}

// recursively executed
FileMeta list(const string &path) {

  // Check if the folder exists
  if (!fs::exists(path) || !fs::is_directory(path)) {
    return {};
  }

  fs::path this_path = path;
  auto file_meta = FileMeta{
      .full_path = this_path.string(),
      .file_name = this_path.filename().string(),
      .extension = this_path.extension().string(),
      .size = static_cast<unsigned int>(fs::file_size(this_path)),
      .is_folder = true,
  };

  // Iterate through the directory
  for (const auto &entry: fs::directory_iterator(path)) {
    if (entry.is_regular_file()) { // Only include regular files
      file_meta.childs.push_back(FileMeta{
          .full_path = entry.path().string(),
          .file_name = entry.path().filename().string(),
          .extension = entry.path().extension().string(),
          .size = static_cast<unsigned int>(entry.file_size()),
          .is_folder = false,
      });
    } else if (entry.is_directory()) {
      auto childs = list(entry.path().string());
      file_meta.childs.push_back(childs);
    }
  }

  return file_meta;
}

vector<char> load(const string &path) {
  std::ifstream file(path, std::ios::binary);
  try {
    if (!file) {
      throw std::runtime_error("Failed to open file: " + path);
    }

    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size)) {
      throw std::runtime_error("Failed to read file: " + path);
    }
    return buffer;
  } catch (exception e) {
    file.close();
    throw e;
  }
  return {};
}
} // namespace ale::data::afs
