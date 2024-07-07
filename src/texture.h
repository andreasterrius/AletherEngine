//
// Created by Alether on 7/8/2024.
//

#ifndef TEXTURE_H
#define TEXTURE_H

#include<vector>
#include<glm/glm.hpp>

#include "data/shader.h"

using namespace std;
using namespace glm;


class Texture {
public:
    unsigned int id;

    int width;
    int height;

    // initialize an empty texture
    Texture(int width, int height);

    void replaceData(vector<vector<vec4>> &colorData);

    void replaceData(vector<vec4> &flatColorData);
};

class TextureRenderer {
public:
    unsigned int vao, vbo, ebo;
    Shader shader;

    TextureRenderer();

    void render(Texture &texture);
};

#endif //TEXTURE_H
