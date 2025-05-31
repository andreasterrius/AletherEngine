//
// Created by Alether on 5/18/2025.
//


#include <assimp/scene.h>

#include "assimp/Importer.hpp"

export module skeletal_mesh;

export namespace ale::graphics {

class SkeletalMesh {
public:
  static SkeletalMesh load(std::string path) { Assimp::Importer importer; }
};

}; // namespace ale::graphics
