//
// Created by Alether on 4/17/2024.
//
#include "model.h"
#include <vector>

ale::Model::Model(const string &path, bool gamma) : gammaCorrection(gamma) {
  loadModel(path);
}

Model::Model(vector<LoadedTexture> textures, vector<Mesh> meshes)
    : textures_loaded(textures), meshes(meshes) {}

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
  vector<LoadedTexture> textures;

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
    vertex.Position = vector;

    if (mesh->HasNormals()) {
      vector.x = mesh->mNormals[i].x;
      vector.y = mesh->mNormals[i].y;
      vector.z = mesh->mNormals[i].z;
      vertex.Normal = vector;
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
      vertex.TexCoords = vec;
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
      vertex.TexCoords = glm::vec2(0.0f, 0.0f);

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
  // process materials
  aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
  // we assume a convention for sampler names in the shaders. Each diffuse
  // texture should be named as 'texture_diffuseN' where N is a sequential
  // number ranging from 1 to MAX_SAMPLER_NUMBER. Same applies to other texture
  // as the following list summarizes: diffuse: texture_diffuseN specular:
  // texture_specularN normal: texture_normalN

  // 1. diffuse maps
  vector<LoadedTexture> diffuseMaps =
      loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
  textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
  // 2. specular maps
  vector<LoadedTexture> specularMaps = loadMaterialTextures(
      material, aiTextureType_SPECULAR, "texture_specular");
  textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
  // 3. normal maps
  std::vector<LoadedTexture> normalMaps =
      loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
  textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
  // 4. height maps
  std::vector<LoadedTexture> heightMaps =
      loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
  textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

  BoundingBox boundingBox = BoundingBox(
      vec3(mesh->mAABB.mMin.x, mesh->mAABB.mMin.y, mesh->mAABB.mMin.z),
      vec3(mesh->mAABB.mMax.x, mesh->mAABB.mMax.y, mesh->mAABB.mMax.z));

  // return a mesh object created from the extracted mesh data
  return Mesh(vertices, indices, textures, boundingBox);
}

vector<LoadedTexture> Model::loadMaterialTextures(aiMaterial *mat,
                                                  aiTextureType type,
                                                  string typeName) {
  vector<LoadedTexture> textures;
  for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
    aiString str;
    mat->GetTexture(type, i, &str);
    // check if texture was loaded before and if so, continue to next iteration:
    // skip loading a new texture
    bool skip = false;
    for (unsigned int j = 0; j < textures_loaded.size(); j++) {
      if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
        textures.push_back(textures_loaded[j]);
        skip = true;
        // a texture with the same filepath has already been loaded, continue to
        // next one. (optimization)
        break;
      }
    }
    if (!skip) {
      // if texture hasn't been loaded already, load it
      LoadedTexture texture;
      texture.id = TextureFromFile(str.C_Str(), this->directory);
      texture.type = typeName;
      texture.path = str.C_Str();
      textures.push_back(texture);
      textures_loaded.push_back(texture);
      // store it as texture loaded for entire model, to ensure we won't
      // unnecessary load duplicate textures.
    }
  }
  return textures;
}

unsigned int ale::TextureFromFile(const char *path, const string &directory,
                                  bool gamma) {
  string filename = string(path);
  filename = directory + '/' + filename;

  unsigned int textureID;
  glGenTextures(1, &textureID);

  int width, height, nrComponents;
  unsigned char *data =
      stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
  if (data) {
    GLenum format;
    if (nrComponents == 1)
      format = GL_RED;
    else if (nrComponents == 3)
      format = GL_RGB;
    else if (nrComponents == 4)
      format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  } else {
    std::cout << "Texture failed to load at path: " << path << std::endl;
    stbi_image_free(data);
  }

  return textureID;
}

Model ModelFactory::createCubeModel() {
  // vector<Vertex> vertices;
  vector<Vertex> vertices = vector{
      // back face
      Vertex{vec3(-1.0f, -1.0f, -1.0f), vec3(0.0f, 0.0f, -1.0f),
             vec2(0.0f, 0.0f)}, // bottom-left
      Vertex{vec3(1.0f, 1.0f, -1.0f), vec3(0.0f, 0.0f, -1.0f),
             vec2(1.0f, 1.0f)}, // top-right
      Vertex{vec3(1.0f, -1.0f, -1.0f), vec3(0.0f, 0.0f, -1.0f),
             vec2(1.0f, 0.0f)}, // bottom-right
      Vertex{vec3(1.0f, 1.0f, -1.0f), vec3(0.0f, 0.0f, -1.0f),
             vec2(1.0f, 1.0f)}, // top-right
      Vertex{vec3(-1.0f, -1.0f, -1.0f), vec3(0.0f, 0.0f, -1.0f),
             vec2(0.0f, 0.0f)}, // bottom-left
      Vertex{vec3(-1.0f, 1.0f, -1.0f), vec3(0.0f, 0.0f, -1.0f),
             vec2(0.0f, 1.0f)}, // top-left
      // front face
      Vertex{vec3(-1.0f, -1.0f, 1.0f), vec3(0.0f, 0.0f, 1.0f),
             vec2(0.0f, 0.0f)}, // bottom-left
      Vertex{vec3(1.0f, -1.0f, 1.0f), vec3(0.0f, 0.0f, 1.0f),
             vec2(1.0f, 0.0f)}, // bottom-right
      Vertex{vec3(1.0f, 1.0f, 1.0f), vec3(0.0f, 0.0f, 1.0f),
             vec2(1.0f, 1.0f)}, // top-right
      Vertex{vec3(1.0f, 1.0f, 1.0f), vec3(0.0f, 0.0f, 1.0f),
             vec2(1.0f, 1.0f)}, // top-right
      Vertex{vec3(-1.0f, 1.0f, 1.0f), vec3(0.0f, 0.0f, 1.0f),
             vec2(0.0f, 1.0f)}, // top-left
      Vertex{vec3(-1.0f, -1.0f, 1.0f), vec3(0.0f, 0.0f, 1.0f),
             vec2(0.0f, 0.0f)}, // bottom-left
      // left face
      Vertex{vec3(-1.0f, 1.0f, 1.0f), vec3(-1.0f, 0.0f, 0.0f),
             vec2(1.0f, 0.0f)}, // top-right
      Vertex{vec3(-1.0f, 1.0f, -1.0f), vec3(-1.0f, 0.0f, 0.0f),
             vec2(1.0f, 1.0f)}, // top-left
      Vertex{vec3(-1.0f, -1.0f, -1.0f), vec3(-1.0f, 0.0f, 0.0f),
             vec2(0.0f, 1.0f)}, // bottom-left
      Vertex{vec3(-1.0f, -1.0f, -1.0f), vec3(-1.0f, 0.0f, 0.0f),
             vec2(0.0f, 1.0f)}, // bottom-left
      Vertex{vec3(-1.0f, -1.0f, 1.0f), vec3(-1.0f, 0.0f, 0.0f),
             vec2(0.0f, 0.0f)}, // bottom-right
      Vertex{vec3(-1.0f, 1.0f, 1.0f), vec3(-1.0f, 0.0f, 0.0f),
             vec2(1.0f, 0.0f)}, // top-right
      // right face
      Vertex{vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 0.0f, 0.0f),
             vec2(1.0f, 0.0f)}, // top-left
      Vertex{vec3(1.0f, -1.0f, -1.0f), vec3(1.0f, 0.0f, 0.0f),
             vec2(0.0f, 1.0f)}, // bottom-right
      Vertex{vec3(1.0f, 1.0f, -1.0f), vec3(1.0f, 0.0f, 0.0f),
             vec2(1.0f, 1.0f)}, // top-right
      Vertex{vec3(1.0f, -1.0f, -1.0f), vec3(1.0f, 0.0f, 0.0f),
             vec2(0.0f, 1.0f)}, // bottom-right
      Vertex{vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 0.0f, 0.0f),
             vec2(1.0f, 0.0f)}, // top-left
      Vertex{vec3(1.0f, -1.0f, 1.0f), vec3(1.0f, 0.0f, 0.0f),
             vec2(0.0f, 0.0f)}, // bottom-left
      // bottom face
      Vertex{vec3(-1.0f, -1.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f),
             vec2(0.0f, 1.0f)}, // top-right
      Vertex{vec3(1.0f, -1.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f),
             vec2(1.0f, 1.0f)}, // top-left
      Vertex{vec3(1.0f, -1.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f),
             vec2(1.0f, 0.0f)}, // bottom-left
      Vertex{vec3(1.0f, -1.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f),
             vec2(1.0f, 0.0f)}, // bottom-left
      Vertex{vec3(-1.0f, -1.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f),
             vec2(0.0f, 0.0f)}, // bottom-right
      Vertex{vec3(-1.0f, -1.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f),
             vec2(0.0f, 1.0f)}, // top-right
      // top face
      Vertex{vec3(-1.0f, 1.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f),
             vec2(0.0f, 1.0f)}, // top-left
      Vertex{vec3(1.0f, 1.0f, 1.0f), vec3(0.0f, 1.0f, 0.0f),
             vec2(1.0f, 0.0f)}, // bottom-right
      Vertex{vec3(1.0f, 1.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f),
             vec2(1.0f, 1.0f)}, // top-right
      Vertex{vec3(1.0f, 1.0f, 1.0f), vec3(0.0f, 1.0f, 0.0f),
             vec2(1.0f, 0.0f)}, // bottom-right
      Vertex{vec3(-1.0f, 1.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f),
             vec2(0.0f, 1.0f)}, // top-left
      Vertex{vec3(-1.0f, 1.0f, 1.0f), vec3(0.0f, 1.0f, 0.0f),
             vec2(0.0f, 0.0f)} // bottom-left
  };

  BoundingBox bb(vec3(-1.0f, -1.0f, -1.0f), vec3(1.0f, 1.0f, 1.0f));
  Mesh mesh(vertices, vector<unsigned int>(), vector<LoadedTexture>(), bb);

  return Model(vector<LoadedTexture>(), vector<Mesh>{std::move(mesh)});
}

Model ModelFactory::createSphereModel(float radius) {
  vector<Vertex> vertices;
  int sectorCount = 72;
  int stackCount = 24;
  float PI = pi<float>();

  // clear memory of prev arrays
  std::vector<float> positions;
  std::vector<float> normals;
  std::vector<float> texCoords;

  float x, y, z, xy;                           // vertex position
  float nx, ny, nz, lengthInv = 1.0f / radius; // vertex normal
  float s, t;                                  // vertex texCoord

  float sectorStep = 2 * PI / sectorCount;
  float stackStep = PI / stackCount;
  float sectorAngle, stackAngle;

  for (int i = 0; i <= stackCount; ++i) {
    stackAngle = PI / 2 - i * stackStep; // starting from pi/2 to -pi/2
    xy = radius * cosf(stackAngle);      // r * cos(u)
    z = radius * sinf(stackAngle);       // r * sin(u)

    // add (sectorCount+1) vertices per stack
    // first and last vertices have same position and normal, but different tex
    // coords
    for (int j = 0; j <= sectorCount; ++j) {
      sectorAngle = j * sectorStep; // starting from 0 to 2pi

      // vertex position (x, y, z)
      x = xy * cosf(sectorAngle); // r * cos(u) * cos(v)
      y = xy * sinf(sectorAngle); // r * cos(u) * sin(v)

      // normalized vertex normal (nx, ny, nz)
      nx = x * lengthInv;
      ny = y * lengthInv;
      nz = z * lengthInv;

      // vertex tex coord (s, t) range between [0, 1]
      s = (float)j / sectorCount;
      t = (float)i / stackCount;

      vertices.push_back(Vertex{
          .Position = vec3(x, y, z),
          // .Normal = vec3(nx, ny, nz),
          // .TexCoords = vec2(s, t),
      });
    }
  }

  // generate CCW index list of sphere triangles
  // k1--k1+1
  // |  / |
  // | /  |
  // k2--k2+1
  std::vector<unsigned int> indices;
  std::vector<int> lineIndices;
  int k1, k2;
  for (int i = 0; i < stackCount; ++i) {
    k1 = i * (sectorCount + 1); // beginning of current stack
    k2 = k1 + sectorCount + 1;  // beginning of next stack

    for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
      // 2 triangles per sector excluding first and last stacks
      // k1 => k2 => k1+1
      if (i != 0) {
        indices.push_back(k1);
        indices.push_back(k2);
        indices.push_back(k1 + 1);
      }

      // k1+1 => k2 => k2+1
      if (i != (stackCount - 1)) {
        indices.push_back(k1 + 1);
        indices.push_back(k2);
        indices.push_back(k2 + 1);
      }

      // store indices for lines
      // vertical lines for all stacks, k1 => k2
      lineIndices.push_back(k1);
      lineIndices.push_back(k2);
      if (i != 0) // horizontal lines except 1st stack, k1 => k+1
      {
        lineIndices.push_back(k1);
        lineIndices.push_back(k1 + 1);
      }
    }
  }

  BoundingBox bb(vec3(-radius / 2.0, -radius / 2.0, -radius / 2.0),
                 vec3(radius / 2.0, radius / 2.0, radius / 2.0));
  Mesh mesh(vertices, indices, vector<LoadedTexture>(), bb);

  return Model(vector<LoadedTexture>(), vector<Mesh>{std::move(mesh)});
}
