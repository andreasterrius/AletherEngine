//
// Created by Alether on 4/18/2024.
//

#include "line_renderer.h"
#include <glad/glad.h>
#include "../data/boundingbox.h"
#include "../data/transform.h"
#include "../data/ray.h"
#include "../file_system.h"
#include "../data/model.h"

using namespace ale;
using afs = ale::FileSystem;

LineRenderer::LineRenderer() : lineShader(Shader(
                                   afs::root("src/renderer/lines.vs").c_str(),
                                   afs::root("src/renderer/lines.fs").c_str())),
                               boxShader(Shader(
                                   afs::root("src/renderer/box.vs").c_str(),
                                   afs::root("src/renderer/box.fs").c_str())) {
    glGenVertexArrays(1, &linesVAO);
    glGenBuffers(1, &linesVBO);
    // fill buffer
    glBindBuffer(GL_ARRAY_BUFFER, linesVBO);
    glBufferData(GL_ARRAY_BUFFER, 200000, nullptr, GL_DYNAMIC_DRAW);
    // link vertex attributes
    glBindVertexArray(linesVAO);
    glEnableVertexAttribArray(0); // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(1); // color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) (3 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //for boxes
    Model cube = ModelFactory::createCubeModel();
    glGenVertexArrays(1, &boxVAO);
    glGenBuffers(1, &boxVBO);

    glBindBuffer(GL_ARRAY_BUFFER, boxVBO);
    glBufferData(GL_ARRAY_BUFFER, cube.meshes[0].vertices.size() * sizeof(Vertex),
                 cube.meshes[0].vertices.data(), GL_STATIC_DRAW);

    glBindVertexArray(boxVAO);
    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) 0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, TexCoords));
    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, Tangent));
    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, Bitangent));
    // ids
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void *) offsetof(Vertex, m_BoneIDs));
    // weights
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, m_Weights));

    // instancing
    glGenBuffers(1, &boxInstanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, boxInstanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 10000, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * 2, (void *) 0);

    glEnableVertexAttribArray(8);
    glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * 2, (void *) sizeof(glm::vec3));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(7, 1);
    glVertexAttribDivisor(8, 1);
    glBindVertexArray(0);
}

void LineRenderer::queueLine(Ray &ray, vec3 color) {
    this->queueLine(ray.origin, ray.dir * vec3(1000), color);
}

void LineRenderer::queueLine(vec3 start, vec3 end, vec3 color) {
    this->lineData.emplace_back(Data{start, color});
    this->lineData.emplace_back(Data{end, color});
}

void LineRenderer::render(mat4 projection, mat4 view) {
    if (!lineData.empty()) {
        this->lineShader.use();
        this->lineShader.setMat4("view", view);
        this->lineShader.setMat4("projection", projection);
        glBindBuffer(GL_ARRAY_BUFFER, linesVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, lineData.size() * sizeof(Data), this->lineData.data());

        glBindVertexArray(linesVAO);
        glDrawArrays(GL_LINES, 0, lineData.size());
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        this->lineData.clear();
    }

    if (!boxData.empty()) {
        this->boxShader.use();
        this->boxShader.setMat4("view", view);
        this->boxShader.setMat4("projection", projection);
        glBindBuffer(GL_ARRAY_BUFFER, boxInstanceVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, boxData.size() * sizeof(vec3), this->boxData.data());

        glBindVertexArray(boxVAO);
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        glDrawArraysInstanced(GL_TRIANGLES, 0, 36, boxData.size());
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        this->boxData.clear();
    }
}

void LineRenderer::queueBox(Transform transform, BoundingBox bb) {
    BoundingBox bbT = bb;
    bbT.min = transform.getModelMatrix() * vec4(bb.min, 1.0);
    bbT.max = transform.getModelMatrix() * vec4(bb.max, 1.0);
    this->boxData.push_back(bbT.min);
    this->boxData.push_back(bbT.max);
}
