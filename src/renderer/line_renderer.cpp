//
// Created by Alether on 4/18/2024.
//

#include "line_renderer.h"
#include <glad/glad.h>
#include "../data/boundingbox.h"
#include "../data/transform.h"
#include "../data/ray.h"
#include "../file_system.h"

using namespace ale;
using afs = ale::FileSystem;

LineRenderer::LineRenderer() : shader(Shader(
        afs::root("src/renderer/lines.vs").c_str(),
        afs::root("src/renderer/lines.fs").c_str())
) {
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
    glGenVertexArrays(1, &boxVAO);
    glGenBuffers(1, &boxVBO);

    glBindBuffer(GL_ARRAY_BUFFER, boxVBO);
    glBufferData(GL_ARRAY_BUFFER, 2000, nullptr, GL_DYNAMIC_DRAW);

    glBindVertexArray(boxVAO);
    glEnableVertexAttribArray(0); // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(1); // color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) (3 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
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
    if(!lineData.empty()) {
        this->shader.use();
        this->shader.setMat4("view", view);
        this->shader.setMat4("projection", projection);
        glBindBuffer(GL_ARRAY_BUFFER, linesVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, lineData.size() * sizeof(Data), this->lineData.data());

        glBindVertexArray(linesVAO);
        glDrawArrays(GL_LINES, 0, lineData.size());
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        this->lineData.clear();
    }

    if(!boxData.empty()) {
        this->shader.use();
        this->shader.setMat4("view", view);
        this->shader.setMat4("projection", projection);
        glBindBuffer(GL_ARRAY_BUFFER, boxVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, boxData.size() * sizeof(Data), this->boxData.data());

        glBindVertexArray(boxVAO);
        glDrawArrays(GL_LINES, 0, boxData.size());
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        this->boxData.clear();
    }
}

void LineRenderer::queueBox(Transform transform, BoundingBox bb) {
    BoundingBox bbT = bb;
    bbT.min = transform.getModelMatrix() * vec4(bb.min, 1.0);
    bbT.max = transform.getModelMatrix() * vec4(bb.max, 1.0);

    

    // bottom point
//    vec3 a = vec3(bbT.min.x, bbT.min.y, bbT.min.z);
//    vec3 b = vec3(bbT.min.x, bbT.min.y, bbT.max.z);
//    vec3 c = vec3(bbT.max.x, bbT.min.y, bbT.max.z);
//    vec3 d = vec3(bbT.max.x, bbT.min.y, bbT.min.z);
//
//    // top points
//    vec3 e = vec3(bbT.min.x, bbT.max.y, bbT.min.z);
//    vec3 f = vec3(bbT.min.x, bbT.max.y, bbT.max.z);
//    vec3 g = vec3(bbT.max.x, bbT.max.y, bbT.max.z);
//    vec3 h = vec3(bbT.max.x, bbT.max.y, bbT.min.z);
//
//    this->queueLine(a, b);
//    this->queueLine(b, c);
//    this->queueLine(c, d);
//    this->queueLine(d, a);
//
//    this->queueLine(a, e);
//    this->queueLine(b, f);
//    this->queueLine(c, g);
//    this->queueLine(d, h);
//
//    this->queueLine(e, f);
//    this->queueLine(f, g);
//    this->queueLine(g, h);
//    this->queueLine(h, e);
}
