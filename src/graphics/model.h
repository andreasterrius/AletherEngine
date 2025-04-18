#ifndef MODEL_H
#define MODEL_H

#include "mesh.h"
#include <assimp/scene.h>
#include <filesystem>
#include <string>
#include <vector>

namespace ale {
using namespace std;

unsigned int TextureFromFile(const char *path, const string &directory,
                             bool gamma = false);

class Model {
public:
  // model data
  vector<Mesh> meshes;
  string id;
  string directory;
  std::filesystem::path path;
  bool gammaCorrection;

  // constructor, expects a filepath to a 3D model.
  Model(string const &path, bool gamma = false);

  Model(vector<Mesh> meshes);

  // draws the model, and thus all its meshes
  void draw(Shader &shader);

private:
  // loads a model with supported ASSIMP extensions from file and stores the
  // resulting meshes in the meshes vector.
  void loadModel(string const &path);

  // processes a node in a recursive fashion. Processes each individual mesh
  // located at the node and repeats this process on its children nodes (if
  // any).
  void processNode(aiNode *node, const aiScene *scene);

  Mesh processMesh(aiMesh *mesh, const aiScene *scene);

  // checks all material textures of a given type and loads the textures if
  // they're not loaded yet. the required info is returned as a Texture struct.
  // vector<PendingTexturePath>
  // loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName);
};
} // namespace ale

#endif
