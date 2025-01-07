#ifndef MODEL_H
#define MODEL_H

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glad/glad.h>
#include <stb_image.h>

#include <assimp/Importer.hpp>
#include <filesystem>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "boundingbox.h"
#include "mesh.h"

using namespace std;
using namespace ale;

namespace ale {

unsigned int TextureFromFile(const char *path, const string &directory,
                             bool gamma = false);

class Model {
public:
  // model data
  vector<LoadedTexture>
      textures_loaded; // stores all the textures loaded so far, optimization
                       // to make sure textures aren't loaded more than once.
  vector<Mesh> meshes;
  string directory;
  std::filesystem::path path;
  bool gammaCorrection;

  // constructor, expects a filepath to a 3D model.
  Model(string const &path, bool gamma = false);

  Model(vector<LoadedTexture> textures, vector<Mesh> meshes);

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
  vector<LoadedTexture>
  loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName);
};

class ModelFactory {
public:
  static Model createCubeModel();

  // https://www.songho.ca/opengl/gl_sphere.html#sphere
  static Model createSphereModel(float radius);
};

} // namespace ale

#endif
