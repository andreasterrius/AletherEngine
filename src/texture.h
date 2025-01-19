//
// Created by Alether on 7/8/2024.
//

#ifndef TEXTURE_H
#define TEXTURE_H

#include <glm/glm.hpp>
#include <stb_image.h>
#include <vector>

#include "data/shader.h"

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
    int min_filter = GL_LINEAR;
    int max_filter = GL_LINEAR;
  };

  Meta meta;
  unsigned int id;

  Texture(string path);
  Texture(Meta meta, vector<float> &data);
  Texture(Meta meta, void *data);
  ~Texture();

  Texture(const Texture &other) = delete;
  Texture &operator=(const Texture &other) = delete;

  Texture(Texture &&other);
  Texture &operator=(Texture &&other);

  void replace_data(vector<vector<vec4>> &color_data);

  void replace_data(vector<vec4> &flat_color_data);

  void partial_replace_data_f32(int xoffset, int yoffset, int width, int height,
                                vector<float> &color_data);

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
  template <typename T> Texture3D(Meta meta, vector<T> &data) : meta(meta) {
    glGenTextures(1, &this->id);
    glBindTexture(GL_TEXTURE_3D, this->id);
    glTexImage3D(GL_TEXTURE_3D, 0, meta.internal_format, meta.width,
                 meta.height, meta.depth, 0, meta.input_format, meta.input_type,
                 data.empty() ? nullptr : data.data());

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_3D, 0);
  }

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
  struct RenderMeta {
    bool discard_alpha = false;
  };
  unsigned int vao, vbo, ebo;
  Shader shader;

  TextureRenderer();
  ~TextureRenderer();

  TextureRenderer(const TextureRenderer &other) = delete;
  TextureRenderer &operator=(const TextureRenderer &other) = delete;

  TextureRenderer(TextureRenderer &&other);
  TextureRenderer &operator=(TextureRenderer &&other);

  void render(Texture &texture, RenderMeta render_meta = {});
};

#endif // TEXTURE_H
