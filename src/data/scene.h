#ifndef SCENE_H
#define SCENE_H

#include <any>
#include <memory>
#include <vector>

using namespace std;

namespace ale {
class SceneNode {
public:
  SceneNode() {};

  shared_ptr<std::any> data;
  vector<SceneNode> childs;
};
} // namespace ale

#endif