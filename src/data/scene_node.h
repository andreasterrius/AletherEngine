//
// Created by Alether on 1/13/2025.
//

#ifndef NAME_H
#define NAME_H
#include <string>

class SceneNode {
  std::string name;

public:
  SceneNode(std::string name);

  std::string get_name();
};

#endif // NAME_H
