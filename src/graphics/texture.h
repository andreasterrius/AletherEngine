//
// Created by Alether on 7/8/2024.
//

#ifndef TEXTURE_H
#define TEXTURE_H

#include "shader.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>

class TextureRendererException final : public std::runtime_error {
public:
  explicit TextureRendererException(const std::string &msg)
      : std::runtime_error(msg) {}
};

class TextureException final : public std::runtime_error {
public:
  explicit TextureException(const std::string &msg) : std::runtime_error(msg) {}
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
  unsigned int id = 0;

  Texture(std::string path);
  Texture(Meta meta, std::vector<float> &data);
  Texture(Meta meta, void *data);
  ~Texture();

  Texture(const Texture &other) = delete;
  Texture &operator=(const Texture &other) = delete;

  Texture(Texture &&other) noexcept;
  Texture &operator=(Texture &&other) noexcept;

  void replace_data(std::vector<std::vector<glm::vec4>> &color_data);

  void replace_data(std::vector<glm::vec4> &flat_color_data);

  void partial_replace_data_f32(int xoffset, int yoffset, int width, int height,
                                std::vector<float> &color_data);

  std::vector<float> retrieve_data_from_gpu();

  void dump_data_to_file(std::string path);
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
  template <typename T>
  Texture3D(Meta meta, std::vector<T> &data) : meta(meta) {
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

  Texture3D(Texture3D &&other) noexcept;
  Texture3D &operator=(Texture3D &&other) noexcept;

  std::vector<float> retrieve_data_from_gpu();
  int get_index(int x, int y, int z, int ele_count);

  void save(std::string name);

  void save_textfile(std::string name);

  static Texture3D load(std::string name);
};

class TextureRenderer {
public:
  struct RenderMeta {
    bool discard_alpha = false;
    bool disable_blending = false;
  };
  unsigned int vao{}, vbo{}, ebo{};
  Shader shader;

  TextureRenderer();
  ~TextureRenderer();

  TextureRenderer(const TextureRenderer &other) = delete;
  TextureRenderer &operator=(const TextureRenderer &other) = delete;

  TextureRenderer(TextureRenderer &&other) noexcept;
  TextureRenderer &operator=(TextureRenderer &&other) noexcept;

  void render(Texture &texture, RenderMeta render_meta = {});
  void render_quad(Shader &override_shader);
};

#endif // TEXTURE_H
