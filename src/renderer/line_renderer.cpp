//
// Created by Alether on 4/18/2024.
//

#include "line_renderer.h"
#include <glad/glad.h>
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
    glBufferData(GL_ARRAY_BUFFER, 2048, nullptr, GL_DYNAMIC_DRAW);
    // link vertex attributes
    glBindVertexArray(linesVAO);
    glEnableVertexAttribArray(0); // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(1); // color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) (3 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void LineRenderer::queue(Ray &ray, vec3 color) {
    this->queue(ray.origin, ray.dir * vec3(1000), color);
}

void LineRenderer::queue(vec3 start, vec3 end, vec3 color) {
    this->data.emplace_back(Data{start, color});
    this->data.emplace_back(Data{end, color});
}

void LineRenderer::render(mat4 projection, mat4 view) {
    if(!data.empty()) {
        this->shader.use();
        this->shader.setMat4("view", view);
        this->shader.setMat4("projection", projection);
        glBindBuffer(GL_ARRAY_BUFFER, linesVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, data.size() * sizeof(Data), this->data.data());

//        cout << "sz:" << posAndColor.size() * sizeof(posAndColor[0]) << endl;

        glBindVertexArray(linesVAO);
        glDrawArrays(GL_LINES, 0, data.size());
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        this->data.clear();
    }
}
