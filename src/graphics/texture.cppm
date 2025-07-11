//
// Created by Alether on 7/8/2024.
//
module;

#include <fstream>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <stb_image.h>
#include <string>
#include <vector>

export module graphics:texture;
import data;
import :shader;

using namespace glm;
using namespace std;
using namespace ale::data;

export namespace ale::graphics {
class TextureRendererException final : public std::runtime_error {
public:
  explicit TextureRendererException(const std::string &msg) :
      std::runtime_error(msg) {}
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

  explicit Texture(string path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    stbi_set_flip_vertically_on_load(true);

    int width, height, nrComponents;
    unsigned char *data =
        stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
    if (data == nullptr) {
      throw TextureException("failed to load image: " + path);
    }

    GLint format;
    if (nrComponents == 1)
      format = GL_RED;
    else if (nrComponents == 2)
      format = GL_RG;
    else if (nrComponents == 3)
      format = GL_RGB;
    else if (nrComponents == 4)
      format = GL_RGBA;
    else
      throw TextureException("nr components not supported");

    *this = Texture(
        Meta{
            .width = width,
            .height = height,
            .internal_format = format,
            .input_format = format,
            .input_type = GL_UNSIGNED_BYTE,
        },
        data);
    stbi_image_free(data);
  }
  Texture(Meta meta, vector<float> &pixels) :
      Texture(meta, pixels.empty() ? nullptr : pixels.data()) {}
  Texture(Meta meta, void *data) : meta(meta) {
    glGenTextures(1, &this->id);
    glBindTexture(GL_TEXTURE_2D, this->id);
    glTexImage2D(GL_TEXTURE_2D, 0, meta.internal_format, meta.width,
                 meta.height, 0, meta.input_format, meta.input_type, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, meta.min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, meta.max_filter);

    glBindTexture(GL_TEXTURE_2D, 0);
  }
  ~Texture() { glDeleteTextures(1, &this->id); }

  Texture(const Texture &other) = delete;
  Texture &operator=(const Texture &other) = delete;

  Texture(Texture &&other) noexcept :
      meta(std::move(other.meta)),
      id(other.id) {
    other.id = 0;
  }
  Texture &operator=(Texture &&other) noexcept {
    if (this != &other) {
      swap(this->id, other.id);
      swap(this->meta, other.meta);
    }
    return *this;
  }

  void replace_data(vector<vector<vec4>> &color_data) {
    // Flatten the 2D vector to a 1D array
    std::vector<glm::vec4> flattenedData;
    for (const auto &row: color_data) {
      flattenedData.insert(flattenedData.end(), row.begin(), row.end());
    }

    this->replace_data(flattenedData);
  }

  void replace_data(vector<vec4> &flat_color_data) {
    glBindTexture(GL_TEXTURE_2D, this->id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, meta.width, meta.height,
                    meta.input_format, meta.input_type, flat_color_data.data());
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  void partial_replace_data_f32(int xoffset, int yoffset, int width, int height,
                                vector<float> &color_data) {
    glBindTexture(GL_TEXTURE_2D, this->id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, width, height,
                    meta.input_format, meta.input_type, color_data.data());
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  std::vector<float> retrieve_data_from_gpu() {
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

  void dump_data_to_file(string path) {
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
  template<typename T>
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

  ~Texture3D() { glDeleteTextures(1, &id); }

  Texture3D(const Texture3D &other) = delete;
  Texture &operator=(const Texture &other) = delete;

  Texture3D(Texture3D &&other) noexcept : meta(other.meta), id(other.id) {
    other.id = 0;
  }
  Texture3D &operator=(Texture3D &&other) noexcept {
    if (this != &other) {
      swap(this->id, other.id);
      swap(this->meta, other.meta);
    }

    return *this;
  }

  std::vector<float> retrieve_data_from_gpu() {
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

  int get_index(int x, int y, int z, int ele_count) {
    return (z * meta.height * meta.width + y * meta.width + x) + ele_count;
  }

  void save(string name) {
    string path = "temp/debug/" + name + ".bin";
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

  void save_textfile(string name) {
    string path = "temp/debug/" + name + ".txt";
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

  static Texture3D load(string name) {
    string path = "temp/debug/" + name + ".bin";
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
};

class TextureRenderer {
public:
  struct RenderMeta {
    bool discard_alpha = false;
    bool enable_blending = true;
  };
  unsigned int vao{}, vbo{}, ebo{};
  Shader shader;

  TextureRenderer() :
      shader(afs::root("resources/shaders/renderer/texture2d.vs").c_str(),
             afs::root("resources/shaders/renderer/texture2d.fs").c_str()) {
    float vertices[] = {
        // Positions       // Texture Coords
        -1.0f, 1.0f,  0.0f, 0.0f, 1.0f, // Top-left
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // Bottom-left
        1.0f,  -1.0f, 0.0f, 1.0f, 0.0f, // Bottom-right
        1.0f,  1.0f,  0.0f, 1.0f, 1.0f // Top-right
    };
    unsigned int indices[] = {
        0, 1, 2, // First triangle
        2, 3, 0 // Second triangle
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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          static_cast<void *>(nullptr));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          reinterpret_cast<void *>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    if (vao == 0 || vbo == 0 || ebo == 0) {
      throw TextureRendererException("failed to initialize texture renderer");
    }
  }
  ~TextureRenderer() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
  }

  TextureRenderer(const TextureRenderer &other) = delete;
  TextureRenderer &operator=(const TextureRenderer &other) = delete;

  TextureRenderer(TextureRenderer &&other) noexcept :
      vao(other.vao),
      vbo(other.vbo),
      ebo(other.ebo),
      shader(std::move(other.shader)) {
    other.vao = 0;
    other.vbo = 0;
    other.ebo = 0;
  }
  TextureRenderer &operator=(TextureRenderer &&other) noexcept {
    if (this != &other) {
      swap(this->vao, other.vao);
      swap(this->vbo, other.vbo);
      swap(this->ebo, other.ebo);
      this->shader = std::move(other.shader);
    }
    return *this;
  }

  void render(Texture &texture, RenderMeta render_meta) {
    shader.use();
    shader.setBool("discard_alpha", render_meta.discard_alpha);

    if (render_meta.enable_blending) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    this->shader.setTexture2D("texture1", 0, texture.id);
    this->render_quad(this->shader);

    if (render_meta.enable_blending) {
      glDisable(GL_BLEND);
    }
  }

  void render_quad(Shader &override_shader) {
    override_shader.use();

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
  }
};
} // namespace ale::graphics
