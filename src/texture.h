//
// Created by Alether on 7/8/2024.
//

#ifndef TEXTURE_H
#define TEXTURE_H

#include "data/shader.h"
#include <glm/glm.hpp>
#include <vector>

using namespace std;
using namespace glm;

class TextureException final : public runtime_error {
public:
  explicit TextureException(const string &msg) : runtime_error(msg) {}
};

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

  Texture(Meta meta, vector<float> &data);
  ~Texture();

  Texture(const Texture &other) = delete;
  Texture &operator=(const Texture &other) = delete;

  Texture(Texture &&other);
  Texture &operator=(Texture &&other);

  void replaceData(vector<vector<vec4>> &colorData);

  void replaceData(vector<vec4> &flatColorData);

  vector<float> retrieve_data_from_gpu();

  void dump_data_to_file(string path);
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
  Texture3D(Meta meta, vector<float> &data);
  ~Texture3D();

  Texture3D(const Texture3D &other) = delete;
  Texture &operator=(const Texture &other) = delete;

  Texture3D(Texture3D &&other);
  Texture3D &operator=(Texture3D &&other);

  vector<float> retrieve_data_from_gpu();

  void save(string name);

  void save_textfile(string name);

  static Texture3D load(string name);
};

class TextureRenderer {
public:
  unsigned int vao, vbo, ebo;
  Shader shader;

  TextureRenderer();
  ~TextureRenderer();

  TextureRenderer(const TextureRenderer &other) = delete;
  TextureRenderer &operator=(const TextureRenderer &other) = delete;

  TextureRenderer(TextureRenderer &&other);
  TextureRenderer &operator=(TextureRenderer &&other);

  void render(Texture &texture);
};

#endif // TEXTURE_H
