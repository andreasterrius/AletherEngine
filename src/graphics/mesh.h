#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <optional>
#include <string>
#include <vector>

import bounding_box;
import shader;

#define MAX_BONE_INFLUENCE 4

using namespace ale::graphics;

namespace ale {
using namespace data;

struct Vertex {
  alignas(16) glm::vec3 position;
  alignas(16) glm::vec3 normal;
  alignas(16) glm::vec2 tex_coords;
  alignas(16) glm::vec3 tangent;
  alignas(16) glm::vec3 bitangent;

  // bone indexes which will influence this vertex
  alignas(16) int m_BoneIDs[MAX_BONE_INFLUENCE] = {0};
  // weights from each bone
  alignas(16) float m_Weights[MAX_BONE_INFLUENCE] = {0};
};

struct PendingTexturePath {
  std::optional<std::string> diffuse;
  std::optional<std::string> specular;
};

class Mesh {
public:
  // mesh Data
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  PendingTexturePath textures;
  BoundingBox boundingBox;
  unsigned int VAO;

  // constructor
  Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices,
       PendingTexturePath pending_texture_path, BoundingBox boundingBox) :
      boundingBox(boundingBox) {
    this->vertices = vertices;
    this->indices = indices;
    this->textures = pending_texture_path;

    // now that we have all the required data, set the vertex buffers and its
    // attribute pointers.
    setupMesh();
  }

  // render the mesh
  void Draw(Shader &shader) {

    // draw mesh
    glBindVertexArray(VAO);
    // glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    if (indices.empty()) {
      glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    } else {
      glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()),
                     GL_UNSIGNED_INT, 0);
    }
    glBindVertexArray(0);
  }

private:
  // render data
  unsigned int VBO, EBO;

  // initializes all the buffer objects/arrays
  void setupMesh() {
    // create buffers/arrays
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // A great thing about structs is that their memory layout is sequential for
    // all its items. The effect is that we can simply pass a pointer to the
    // struct and it translates perfectly to a glm::vec3/2 array which again
    // translates to 3/2 floats which translates to a byte array.
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
                 &vertices[0], GL_STATIC_DRAW);

    // if there are indices, then let's make an EBO
    if (!indices.empty()) {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                   indices.size() * sizeof(unsigned int), &indices[0],
                   GL_STATIC_DRAW);
    }

    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) 0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *) offsetof(Vertex, normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *) offsetof(Vertex, tex_coords));
    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *) offsetof(Vertex, tangent));
    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *) offsetof(Vertex, bitangent));
    // bone ids
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex),
                           (void *) offsetof(Vertex, m_BoneIDs));
    // weights
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *) offsetof(Vertex, m_Weights));

    //        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // THIS IS NOT ALLOWED,
    //        THIS WILL UNBOUND EBO FROM VAO;
    glBindBuffer(GL_ARRAY_BUFFER, 0); // THIS IS ALLOWED.
    glBindVertexArray(0);
  }
};
} // namespace ale

#endif
