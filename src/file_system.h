//
// Created by Alether on 4/16/2024.
//

#ifndef ALETHERENGINE_FILE_SYSTEM_H
#define ALETHERENGINE_FILE_SYSTEM_H

#include "config.h"
#include <string>
#include <vector>

namespace ale {
struct FileMeta {
  std::string full_path;
  std::string file_name;
  std::string extension;
  unsigned int size;
};

class FileSystem {
public:
  static std::string root(const std::string &path);

  static std::string from_root(const std::string &path);

  static std::vector<FileMeta> list(const std::string &path);
};

} // namespace ale

#endif // ALETHERENGINE_FILE_SYSTEM_H
