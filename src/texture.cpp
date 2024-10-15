//
// Created by Alether on 7/8/2024.
//

#include "texture.h"
#include <glad/glad.h>
#include "file_system.h"

using afs = ale::FileSystem;

TextureRenderer::TextureRenderer() : shader(afs::root("src/shaders/texture2d.vs").c_str(),
                                            afs::root("src/shaders/texture2d.fs").c_str()) {
    float vertices[] = {
        // Positions       // Texture Coords
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, // Top-left
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // Bottom-left
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // Bottom-right
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f  // Top-right
    };
    unsigned int indices[] = {
        0, 1, 2, // First triangle
        2, 3, 0  // Second triangle
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void TextureRenderer::render(Texture& texture) {
    shader.use();
    shader.setInt("texture1", 0);
    glActiveTexture(GL_TEXTURE0 + 0); // active proper texture unit before binding
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

Texture::Texture(Meta meta) : meta(meta) {
    vector<vec4> emptyPixel(meta.width * meta.height, vec4(0.0));

    glGenTextures(1, &this->id);
    glBindTexture(GL_TEXTURE_2D, this->id);
    glTexImage2D(GL_TEXTURE_2D, 0, meta.internal_format,
                 meta.width, meta.height, 0, meta.input_format, meta.input_type, emptyPixel.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::replaceData(vector<vector<vec4>>& colorData) {
    // Flatten the 2D vector to a 1D array
    std::vector<glm::vec4> flattenedData;
    for (const auto& row : colorData){
        flattenedData.insert(flattenedData.end(), row.begin(), row.end());
    }

    this->replaceData(flattenedData);
}

void Texture::replaceData(vector<vec4>& flatColorData) {
    glBindTexture(GL_TEXTURE_2D, this->id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, meta.width, meta.height, meta.input_format, meta.input_type,
                    flatColorData.data());
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture3D::Texture3D(Meta meta, vector<float> *data) : meta(meta) {

    glGenTextures(1, &this->id);
    glBindTexture(GL_TEXTURE_3D, this->id);
    if(data != nullptr){
        glTexImage3D(GL_TEXTURE_3D, 0, meta.internal_format,
                     meta.width, meta.height, meta.depth, 0, meta.input_format, meta.input_type, data->data());
    } else{
        vector<vec4> empty_pixel(meta.width * meta.height * meta.depth, vec4(0.0));
        glTexImage3D(GL_TEXTURE_3D, 0, meta.internal_format,
                    meta.width, meta.height, meta.depth, 0, meta.input_format, meta.input_type, empty_pixel.data());
    }

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_3D, 0);
}

vector<float> Texture3D::dump_data_from_gpu(){
    unsigned long long data_size = this->meta.width * this->meta.height * this->meta.depth;
    if(this->meta.input_format == GL_RGBA) {
        data_size *= 4;
    }
    if(this->meta.input_type == GL_FLOAT) {
        data_size *= sizeof(float);
    }
    vector<float> data(data_size);

    glBindTexture(GL_TEXTURE_3D, this->id);
    glGetTexImage(GL_TEXTURE_3D, 0, this->meta.input_format, this->meta.input_type, data.data());
    glBindTexture(GL_TEXTURE_3D, 0);
    
    return data;
}
