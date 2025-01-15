//
// Created by Alether on 1/13/2025.
//

#include "scene_node.h"

SceneNode::SceneNode(std::string name) : name(name) {}

std::string SceneNode::get_name() { return this->name; }