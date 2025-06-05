//
// Created by Alether on 4/17/2024.
//
#include "model.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

namespace ale {

Model::Model(const string &path, bool gamma) : gammaCorrection(gamma) {
  loadModel(path);
}
Model::Model(vector<Mesh> meshes) : meshes(meshes) {}

void Model::draw(Shader &shader) {
  for (unsigned int i = 0; i < meshes.size(); i++)
    meshes[i].Draw(shader);
}

void Model::loadModel(const string &path) {
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

void Model::processNode(aiNode *node, const aiScene *scene) {
  // process each mesh located at the current node
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    // the node object only contains indices to index the actual objects in the
    // scene. the scene contains all the data, node is just to keep stuff
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

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene) {
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
    // that doesn't directly convert to glm's vec3 class so we transfer the data
    // to this placeholder glm::vec3 first. positions
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

  // // bones + weights
  // for (unsigned int i = 0; i < mesh->mNumBones; i++) {
  //   aiBone *bone = mesh->mBones[i];
  //   string bone_name = bone->mName.data;
  //
  //   int bone_index = 0;
  //   if (bone_mapping.find(bone_name) == bone_mapping.end()) {
  //     bone_index = bone_counter++;
  //     bone_mapping[bone_name] = bone_index;
  //     bone_offset_matrices.push_back(
  //         glm::transpose(glm::make_mat4(&bone->mOffsetMatrix.a1)));
  //   } else {
  //     bone_index = bone_mapping[bone_name];
  //   }
  //
  //   for (unsigned int j = 0; j < bone->mNumWeights; j++) {
  //     unsigned int vertex_id = bone->mWeights[j].mVertexId;
  //     float weight = bone->mWeights[j].mWeight;
  //
  //     for (int k = 0; k < 4; ++k) {
  //       if (vertices[vertex_id].m_Weights[k] == 0.0f) {
  //         vertices[vertex_id].m_BoneIDs[k] = bone_index;
  //         vertices[vertex_id].m_Weights[k] = weight;
  //         break;
  //       }
  //     }
  //   }
  // }

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

} // namespace ale
