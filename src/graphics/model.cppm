module;

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <filesystem>
#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

export module graphics:model;
import data;
import :mesh;
import :shader;

export namespace ale::graphics {
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
  Model(string const &path, bool gamma = false) : gammaCorrection(gamma) {
    loadModel(path);
  }

  Model(vector<Mesh> meshes) : meshes(meshes) {}

  // draws the model, and thus all its meshes
  void draw(Shader &shader) {
    for (unsigned int i = 0; i < meshes.size(); i++)
      meshes[i].Draw(shader);
  }

private:
  // loads a model with supported ASSIMP extensions from file and stores the
  // resulting meshes in the meshes vector.
  void loadModel(const string &path) {
    // read file via ASSIMP
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(
        path, aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                  aiProcess_FlipUVs | aiProcess_CalcTangentSpace |
                  aiProcess_GenBoundingBoxes);
    // check for errors
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
        !scene->mRootNode) // if is Not Zero
    {
      cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
      return;
    }
    // retrieve the directory path of the filepath
    directory = path.substr(0, path.find_last_of('/'));
    this->path = path;

    // process ASSIMP's root node recursively
    processNode(scene->mRootNode, scene);
  }

  // processes a node in a recursive fashion. Processes each individual mesh
  // located at the node and repeats this process on its children nodes (if
  // any).
  void processNode(aiNode *node, const aiScene *scene) {
    // process each mesh located at the current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
      // the node object only contains indices to index the actual objects in
      // the scene. the scene contains all the data, node is just to keep stuff
      // organized (like relations between nodes).
      aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
      meshes.push_back(processMesh(mesh, scene));
    }
    // after we've processed all of the meshes (if any) we then recursively
    // process each of the children nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
      processNode(node->mChildren[i], scene);
    }
  }

  Mesh processMesh(aiMesh *mesh, const aiScene *scene) {
    // data to fill
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<PendingTexturePath> textures;

    // unordered_map<string, int> bone_mapping;
    // vector<glm::mat4> bone_offset_matrices;
    // int bone_counter = 0;

    // walk through each of the mesh's vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
      Vertex vertex;
      glm::vec3 vector;
      // we declare a placeholder vector since assimp uses its own vector class
      // that doesn't directly convert to glm's vec3 class so we transfer the
      // data to this placeholder glm::vec3 first. positions
      vector.x = mesh->mVertices[i].x;
      vector.y = mesh->mVertices[i].y;
      vector.z = mesh->mVertices[i].z;
      vertex.position = vector;

      if (mesh->HasNormals()) {
        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.normal = vector;
      }
      // texture coordinates
      if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
      {
        glm::vec2 vec;
        // a vertex can contain up to 8 different texture coordinates. We thus
        // make the assumption that we won't use models where a vertex can have
        // multiple texture coordinates so we always take the first set (0).
        vec.x = mesh->mTextureCoords[0][i].x;
        vec.y = mesh->mTextureCoords[0][i].y;
        vertex.tex_coords = vec;
        // tangent
        vector.x = mesh->mTangents[i].x;
        vector.y = mesh->mTangents[i].y;
        vector.z = mesh->mTangents[i].z;
        // vertex.Tangent = vector;
        // bitangent
        vector.x = mesh->mBitangents[i].x;
        vector.y = mesh->mBitangents[i].y;
        vector.z = mesh->mBitangents[i].z;
        // vertex.Bitangent = vector;
      } else
        vertex.tex_coords = glm::vec2(0.0f, 0.0f);

      for (int j = 0; j < MAX_BONE_INFLUENCE; ++j) {
        vertex.m_BoneIDs[j] = -1;
        vertex.m_Weights[j] = 0.0f;
      }

      vertices.push_back(vertex);
    }
    // now wak through each of the mesh's faces (a face is a mesh its triangle)
    // and retrieve the corresponding vertex indices.
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
      aiFace face = mesh->mFaces[i];
      // retrieve all indices of the face and store them in the indices vector
      for (unsigned int j = 0; j < face.mNumIndices; j++)
        indices.push_back(face.mIndices[j]);
    }

    // bones + weights
    auto bone_mapping = unordered_map<string, BoneInfo>();
    int bone_counter = 0;
    for (unsigned int i = 0; i < mesh->mNumBones; i++) {
      aiBone *bone = mesh->mBones[i];
      string bone_name = bone->mName.data;

      auto bone_index = -1;
      if (!bone_mapping.contains(bone_name)) {
        bone_index = bone_counter++;
        bone_mapping[bone_name] = BoneInfo{
            bone_index, AssimpToGLMMat(mesh->mBones[i]->mOffsetMatrix)};
      } else {
        bone_index = bone_mapping.at(bone_name).id;
      }
      assert(bone_index != -1);
      auto weights = mesh->mBones[bone_index]->mWeights;
      auto num_weights = mesh->mBones[bone_index]->mNumWeights;

      for (int j = 0; j < num_weights; ++j) {
        auto vertex_id = weights[j].mVertexId;
        auto weight = weights[j].mWeight;
        assert(vertex_id < vertices.size());

        // find the first empty position or don't assign at all
        for (int k = 0; k < MAX_BONE_INFLUENCE; ++k) {
          if (vertices[vertex_id].m_BoneIDs[k] < 0) {
            vertices[vertex_id].m_BoneIDs[k] = bone_index;
            vertices[vertex_id].m_Weights[k] = weight;
            break;
          }
        }
      }
    }

    // process materials
    aiMaterial *mat = scene->mMaterials[mesh->mMaterialIndex];

    auto pending_texture_paths = PendingTexturePath();
    if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
      aiString raw_path;
      mat->GetTexture(aiTextureType_DIFFUSE, 0, &raw_path);
      pending_texture_paths.diffuse =
          (path.parent_path() / raw_path.C_Str()).string();
    }

    if (mat->GetTextureCount(aiTextureType_SPECULAR) > 0) {
      aiString raw_path;
      mat->GetTexture(aiTextureType_SPECULAR, 0, &raw_path);
      pending_texture_paths.specular =
          (path.parent_path() / raw_path.C_Str()).string();
    }

    BoundingBox boundingBox = BoundingBox(
        glm::vec3(mesh->mAABB.mMin.x, mesh->mAABB.mMin.y, mesh->mAABB.mMin.z),
        glm::vec3(mesh->mAABB.mMax.x, mesh->mAABB.mMax.y, mesh->mAABB.mMax.z));

    // return a mesh object created from the extracted mesh data
    return Mesh(vertices, indices, pending_texture_paths, boundingBox);
  }

  // names
  mat4 AssimpToGLMMat(aiMatrix4x4 mat) {
    return mat4(mat.a1, mat.a2, mat.a3, mat.a4, mat.b1, mat.b2, mat.b3, mat.b4,
                mat.c1, mat.c2, mat.c3, mat.c4, mat.d1, mat.d2, mat.d3, mat.d4);
  }

  // checks all material textures of a given type and loads the textures if
  // they're not loaded yet. the required info is returned as a Texture
  // struct. vector<PendingTexturePath> loadMaterialTextures(aiMaterial *mat,
  // aiTextureType type, string typeName);
};
} // namespace ale::graphics
