//
// Created by Alether on 1/13/2025.
//

#ifndef NAME_H
#define NAME_H
#include <nlohmann/json.hpp>
#include <string>

namespace ale {
struct SceneNode {
  std::string name;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SceneNode, name);
} // namespace ale
#endif // NAME_H
