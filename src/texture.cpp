//
// Created by Alether on 7/8/2024.
//

#include "texture.h"

#include <glad/glad.h>

#include "file_system.h"

using afs = ale::FileSystem;

TextureRenderer::TextureRenderer()
    : shader(afs::root("src/shaders/texture2d.vs").c_str(),
             afs::root("src/shaders/texture2d.fs").c_str()) {
  float vertices[] = {
      // Positions       // Texture Coords
      -1.0f, 1.0f,  0.0f, 0.0f, 1.0f, // Top-left
      -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // Bottom-left
      1.0f,  -1.0f, 0.0f, 1.0f, 0.0f, // Bottom-right
      1.0f,  1.0f,  0.0f, 1.0f, 1.0f  // Top-right
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
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

TextureRenderer::~TextureRenderer() {
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ebo);
}

TextureRenderer::TextureRenderer(TextureRenderer &&other)
    : vao(other.vao), vbo(other.vbo), shader(other.shader) {
  other.vao = 0;
  other.vbo = 0;
}

TextureRenderer &TextureRenderer::operator=(TextureRenderer &&other) {
  if (this != &other) {
    swap(this->vao, other.vao);
    swap(this->vbo, other.vbo);
    this->shader = std::move(other.shader);
  }

  return *this;
}

void TextureRenderer::render(Texture &texture) {
  shader.use();
  shader.setInt("texture1", 0);
  glActiveTexture(GL_TEXTURE0 + 0); // active proper texture unit before binding
  glBindTexture(GL_TEXTURE_2D, texture.id);
  glBindVertexArray(vao);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

Texture::Texture(Meta meta, vector<float> &pixels) : meta(meta) {
  glGenTextures(1, &this->id);
  glBindTexture(GL_TEXTURE_2D, this->id);
  glTexImage2D(GL_TEXTURE_2D, 0, meta.internal_format, meta.width, meta.height,
               0, meta.input_format, meta.input_type,
               pixels.empty() ? nullptr : pixels.data());

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, meta.min_filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, meta.max_filter);

  glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture() { glDeleteBuffers(1, &this->id); }

void Texture::replace_data(vector<vector<vec4>> &color_data) {
  // Flatten the 2D vector to a 1D array
  std::vector<glm::vec4> flattenedData;
  for (const auto &row : color_data) {
    flattenedData.insert(flattenedData.end(), row.begin(), row.end());
  }

  this->replace_data(flattenedData);
}

void Texture::replace_data(vector<vec4> &flat_color_data) {
  glBindTexture(GL_TEXTURE_2D, this->id);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, meta.width, meta.height,
                  meta.input_format, meta.input_type, flat_color_data.data());
  glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::partial_replace_data_f32(int xoffset, int yoffset, int width,
                                       int height, vector<float> &color_data) {
  glBindTexture(GL_TEXTURE_2D, this->id);
  glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, width, height,
                  meta.input_format, meta.input_type, color_data.data());
  glBindTexture(GL_TEXTURE_2D, 0);
}

vector<float> Texture::retrieve_data_from_gpu() {
  unsigned long long element_size = this->meta.width * this->meta.height;

  switch (this->meta.input_format) {
  case GL_RGBA:
    element_size *= 4;
    break;
  case GL_RGB:
    element_size *= 3;
    break;
  case GL_RG:
    element_size *= 2;
    break;
  case GL_RED:
    element_size *= 1;
    break;
  }
  vector<float> data(element_size);

  glBindTexture(GL_TEXTURE_2D, this->id);
  glGetTexImage(GL_TEXTURE_2D, 0, this->meta.input_format,
                this->meta.input_type, data.data());
  glBindTexture(GL_TEXTURE_2D, 0);

  return data;
}

void Texture::dump_data_to_file(string path) {
  auto data = this->retrieve_data_from_gpu();
  ofstream out_file(path);
  if (out_file.is_open()) {
    int ctr = 0;
    for (int i = 0; i < meta.height; ++i) {
      for (int j = 0; j < meta.width; ++j) {
        if (meta.input_format == GL_RGBA) {
          out_file << "(" << data[ctr] << "," << data[ctr + 1] << ","
                   << data[ctr + 2] << "," << data[ctr + 3] << ") ";
          ctr += 4;
        } else if (meta.input_format == GL_RED) {
          out_file << data[ctr] << " ";
          ctr += 1;
        } else {
          throw TextureException("format not yet supported");
        }
      }
      out_file << "\n";
    }
  }
  out_file.close();
}

Texture::Texture(Texture &&other) : meta(std::move(other.meta)), id(other.id) {
  other.id = 0;
}

Texture &Texture::operator=(Texture &&other) {
  if (this != &other) {
    swap(this->id, other.id);
    swap(this->meta, other.meta);
  }

  return *this;
}

Texture3D::~Texture3D() { glDeleteTextures(1, &id); }

vector<float> Texture3D::retrieve_data_from_gpu() {
  unsigned long long element_size =
      this->meta.width * this->meta.height * this->meta.depth;

  switch (this->meta.input_format) {
  case GL_RGBA:
    element_size *= 4;
    break;
  case GL_RGB:
    element_size *= 3;
    break;
  case GL_RG:
    element_size *= 2;
    break;
  case GL_RED:
    element_size *= 1;
    break;
  }
  vector<float> data(element_size);

  glBindTexture(GL_TEXTURE_3D, this->id);
  glGetTexImage(GL_TEXTURE_3D, 0, this->meta.input_format,
                this->meta.input_type, data.data());
  glBindTexture(GL_TEXTURE_3D, 0);

  return data;
}

void Texture3D::save(string name) {
  string path = "resources/" + name + ".bin";
  ofstream out_file(afs::root(path), std::ios::out | std::ios::binary);
  if (!out_file.is_open()) {
    throw TextureException("unable to create file: " + path);
  }

  auto pixels = this->retrieve_data_from_gpu();
  auto pixels_size = pixels.size();

  out_file.write(reinterpret_cast<const char *>(&meta), sizeof(meta));
  out_file.write(reinterpret_cast<const char *>(&pixels_size),
                 sizeof(pixels_size));
  out_file.write(reinterpret_cast<const char *>(pixels.data()),
                 pixels_size * sizeof(float));
  out_file.close();
}

void Texture3D::save_textfile(string name) {
  string path = "resources/" + name + ".txt";
  ofstream out_file(afs::root(path), std::ios::out | std::ios::binary);
  if (out_file.is_open()) {
    auto pixels = this->retrieve_data_from_gpu();
    int ctr = 0;
    for (int i = 0; i < meta.width; ++i) {
      out_file << "i : " << i << endl;
      for (int j = 0; j < meta.height; ++j) {
        for (int k = 0; k < meta.depth; ++k) {
          out_file << pixels[ctr] << " ";
          ctr++;
        }
        out_file << "| j: " << j << endl;
      }
      out_file << endl;
    }
  }
  out_file.close();
}

Texture3D Texture3D::load(string name) {
  string path = "resources/" + name + ".bin";
  ifstream in_file(afs::root(path), std::ios::binary);
  if (!in_file.is_open()) {
    throw TextureException("uanble to load file: " + path);
  }

  Meta meta = {0};
  size_t pixels_size = {0};
  vector<float> pixels;

  in_file.read(reinterpret_cast<char *>(&meta), sizeof(meta));
  in_file.read(reinterpret_cast<char *>(&pixels_size), sizeof(size_t));
  pixels.resize(pixels_size);
  in_file.read(reinterpret_cast<char *>(pixels.data()),
               pixels_size * sizeof(float));
  in_file.close();

  return std::move(Texture3D(meta, pixels));
}

Texture3D::Texture3D(Texture3D &&other) : meta(other.meta), id(other.id) {
  other.id = 0;
}

Texture3D &Texture3D::operator=(Texture3D &&other) {
  if (this != &other) {
    swap(this->id, other.id);
    swap(this->meta, other.meta);
  }

  return *this;
}
