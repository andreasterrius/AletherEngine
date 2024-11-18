//
// Created by Alether on 4/16/2024.
//

#include "file_system.h"

namespace ale {
string FileSystem::root(const string &path) {
  return string(ALE_ROOT_PATH) + "/" + path;
}
} // namespace ale
