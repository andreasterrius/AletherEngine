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
    struct Meta {
        int width = 0;
        int height = 0;
        int internal_format = GL_RGBA;
        int input_format = GL_RGBA;
        int input_type = GL_FLOAT;
    };

    Meta meta;
    unsigned int id;

    // initialize an empty texture
    Texture(Meta meta);

    void replaceData(vector<vector<vec4>>& colorData);

    void replaceData(vector<vec4>& flatColorData);
};

class Texture3D {
public:
    struct Meta {
        int width = 0;
        int height = 0;
        int depth = 0;
        int internal_format = GL_RGBA;
        int input_format = GL_RGBA;
        int input_type = GL_FLOAT;
    };

    Meta meta;
    unsigned int id = 0;

    // initialize a texture, empty data is possible
    Texture3D(Meta meta, vector<float>* data = nullptr);
};

class TextureRenderer {
public:
    unsigned int vao, vbo, ebo;
    Shader shader;

    TextureRenderer();

    void render(Texture& texture);
};

#endif //TEXTURE_H
