module;

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <filesystem>
#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <vector>

export module graphics:animation;
import data;
import :mesh;
import :shader;
using namespace std;

export namespace ale::graphics {
class Animation {};
} // namespace ale::graphics
