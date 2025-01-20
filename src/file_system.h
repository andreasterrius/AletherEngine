//
// Created by Alether on 4/16/2024.
//

#ifndef ALETHERENGINE_FILE_SYSTEM_H
#define ALETHERENGINE_FILE_SYSTEM_H

#include "config.h"
#include <string>
#include <vector>

using namespace std;

namespace ale {
struct FileMeta {
  string full_path;
  string file_name;
  string extension;
  unsigned int size;
};

class FileSystem {
public:
  static string root(const string &path);

  static string from_root(const string &path);

  static vector<FileMeta> list(const string &path);
};

} // namespace ale

#endif // ALETHERENGINE_FILE_SYSTEM_H
