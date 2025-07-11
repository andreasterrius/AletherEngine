//
// Created by Alether on 4/18/2024.
//
module;

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

export module graphics:line_renderer;
import data;
import :model;
import :shader;
import :mesh;
import :ray;

#define LINE_BUFFER_SIZE 300000
#define BOX_BUFFER_SIZE 90000

using namespace ale::graphics;
using namespace ale::data;
using namespace glm;


export namespace ale::graphics {

class LineRenderer {
private:
  struct Data {
    glm::vec3 startPos;
    glm::vec3 color;
  };
  Shader lineShader;
  unsigned int linesVAO, linesVBO;
  vector<Data> lineData;

  Shader boxShader;
  unsigned int boxVAO, boxVBO, boxInstanceVBO;
  vector<glm::vec3> boxData;

  Model create_box() {
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
    Mesh mesh(vertices, vector<unsigned int>(), PendingTexturePath(), bb);

    return Model(vector<Mesh>{std::move(mesh)});
  }

public:
  LineRenderer() :
      lineShader(
          Shader(afs::root("resources/shaders/renderer/lines.vs").c_str(),
                 afs::root("resources/shaders/renderer/lines.fs").c_str())),
      boxShader(
          Shader(afs::root("resources/shaders/renderer/box.vs").c_str(),
                 afs::root("resources/shaders/renderer/box.fs").c_str())) {
    glGenVertexArrays(1, &linesVAO);
    glGenBuffers(1, &linesVBO);
    // fill buffer
    glBindBuffer(GL_ARRAY_BUFFER, linesVBO);
    glBufferData(GL_ARRAY_BUFFER, LINE_BUFFER_SIZE * sizeof(Data), nullptr,
                 GL_DYNAMIC_DRAW);
    // just blit it multiple times bro
    //  link vertex attributes
    glBindVertexArray(linesVAO);
    glEnableVertexAttribArray(0); // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void *) 0);
    glEnableVertexAttribArray(1); // color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void *) (3 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // for boxes
    // Model cube = Model(afs::root("resources/default_models/unit_cube.obj"));
    Model cube = create_box();
    glGenVertexArrays(1, &boxVAO);
    glGenBuffers(1, &boxVBO);

    glBindBuffer(GL_ARRAY_BUFFER, boxVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 cube.meshes[0].vertices.size() * sizeof(Vertex),
                 cube.meshes[0].vertices.data(), GL_STATIC_DRAW);

    glBindVertexArray(boxVAO);
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

    // instancing
    glGenBuffers(1, &boxInstanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, boxInstanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * LINE_BUFFER_SIZE, nullptr,
                 GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * 3,
                          (void *) 0);

    glEnableVertexAttribArray(8);
    glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * 3,
                          (void *) sizeof(glm::vec3));

    glEnableVertexAttribArray(9);
    glVertexAttribPointer(9, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * 3,
                          (void *) (2 * sizeof(vec3)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(7, 1);
    glVertexAttribDivisor(8, 1);
    glVertexAttribDivisor(9, 1);
    glBindVertexArray(0);
  }

  void queue_line(Ray &ray, glm::vec3 color = WHITE) {
    this->queue_line(ray.origin, ray.dir * vec3(1000), color);
  }

  void queue_line(glm::vec3 start, glm::vec3 end, glm::vec3 color = WHITE) {
    this->lineData.emplace_back(Data{start, color});
    this->lineData.emplace_back(Data{end, color});
  }

  void queue_box(Transform transform, BoundingBox bb, glm::vec3 color = WHITE) {
    BoundingBox bbT = bb;
    bbT.min = transform.get_model_matrix() * vec4(bb.min, 1.0);
    bbT.max = transform.get_model_matrix() * vec4(bb.max, 1.0);
    this->boxData.push_back(bbT.min);
    this->boxData.push_back(bbT.max);
    this->boxData.push_back(color);
  }

  void queue_unit_cube(Transform transform) {
    BoundingBox bb(vec3(-0.5f), vec3(0.5f));
    bb.min = transform.get_model_matrix() * vec4(bb.min, 1.0);
    bb.max = transform.get_model_matrix() * vec4(bb.max, 1.0);
    this->boxData.push_back(bb.min);
    this->boxData.push_back(bb.max);
    this->boxData.push_back(WHITE);
  }

  void render(glm::mat4 projection, glm::mat4 view) {
    if (!lineData.empty()) {
      glBindBuffer(GL_ARRAY_BUFFER, linesVBO);
      this->lineShader.use();
      this->lineShader.setMat4("view", view);
      this->lineShader.setMat4("projection", projection);
      for (int i = 0; i < this->lineData.size(); i += LINE_BUFFER_SIZE) {
        int size = std::min(LINE_BUFFER_SIZE, (int) this->lineData.size() - i);
        glBufferSubData(GL_ARRAY_BUFFER, 0, size * sizeof(Data),
                        &this->lineData[i]);

        glBindVertexArray(linesVAO);
        glDrawArrays(GL_LINES, 0, size);
      }

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
      this->lineData.clear();
    }

    if (!boxData.empty()) {
      this->boxShader.use();
      this->boxShader.setMat4("view", view);
      this->boxShader.setMat4("projection", projection);
      glBindBuffer(GL_ARRAY_BUFFER, boxInstanceVBO);
      for (int i = 0; i < this->boxData.size(); i += BOX_BUFFER_SIZE) {
        int size = std::min(BOX_BUFFER_SIZE, (int) this->boxData.size() - i);
        ;

        glBufferSubData(GL_ARRAY_BUFFER, 0, size * sizeof(vec3),
                        &this->boxData[i]);

        glBindVertexArray(boxVAO);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 36, size);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
      }
      glBindVertexArray(0);
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      this->boxData.clear();
    }
  }
};
} // namespace ale::graphics
