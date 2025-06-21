//
// Created by Alether on 5/18/2025.
//
module;

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

export module graphics:skeletal_mesh;

export namespace ale::graphics {

class SkeletalMesh {
public:
  static SkeletalMesh load(std::string path) { return {}; }
};

}; // namespace ale::graphics
