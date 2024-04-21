//
// Created by Alether on 4/18/2024.
//

#ifndef ALETHERENGINE_LINE_RENDERER_H
#define ALETHERENGINE_LINE_RENDERER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "../data/shader.h"
#include <vector>
#include <utility>

using namespace glm;
using namespace std;

static vec3 WHITE = vec3(1.0, 1.0, 1.0);

namespace ale {

class Ray;

class LineRenderer {
private:
    unsigned int linesVAO, linesVBO;
    Shader shader;

    struct Data {
        vec3 startPos;
        vec3 color;
    };

    vector<Data> data;

public:
    LineRenderer();

    void queue(Ray &ray, vec3 color = WHITE);

    void queue(vec3 start, vec3 end, vec3 color = WHITE);

    void render(mat4 projection, mat4 view);
};
}


#endif //ALETHERENGINE_LINE_RENDERER_H
