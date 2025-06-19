module;

#include <fstream>
#include <glad/glad.h>
#include <iostream>
#include <string>
#include "shader_common.h"

export module graphics:compute_shader;
import :texture;
import :shader;

using namespace std;

export namespace ale::graphics {
class ComputeShader {
public:
  unsigned int id;

  ComputeShader(string path) {
    std::ifstream computeShaderFile;
    string computeShaderCode = load_file_with_include(IncludeDirective{
        .filename = path,
    });

    unsigned int compute = glCreateShader(GL_COMPUTE_SHADER);
    const char *cShaderCode = computeShaderCode.c_str();
    glShaderSource(compute, 1, &cShaderCode, NULL);
    glCompileShader(compute);
    checkCompileErrors(compute, "COMPUTE", path);

    this->id = glCreateProgram();
    glAttachShader(id, compute);
    glLinkProgram(id);
    checkCompileErrors(id, "PROGRAM", "");
  }
  ~ComputeShader() { glDeleteProgram(id); }

  ComputeShader(const ComputeShader &other) = delete;
  ComputeShader &operator=(const ComputeShader &other) = delete;

  ComputeShader(ComputeShader &&other) { other.id = 0; }
  ComputeShader &operator=(ComputeShader &&other) {
    if (this != &other) {
      swap(this->id, other.id);
    }

    return *this;
  }

  void execute_2d_save_to_texture_2d(Texture &texture) {
    glUseProgram(this->id);
    // TODO: this does not need to be bound inside hot loop actually. but
    // convinent
    glBindImageTexture(0, texture.id, 0, GL_FALSE, 0, GL_READ_WRITE,
                       texture.meta.internal_format);
    glDispatchCompute(texture.meta.width, texture.meta.height, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT |
                    GL_TEXTURE_UPDATE_BARRIER_BIT);
  }

  void execute_3d_save_to_texture_3d(Texture3D &texture) {
    glUseProgram(this->id);
    // TODO: this does not need to be bound inside hot loop actually. but
    // convinent
    glBindImageTexture(0, texture.id, 0, GL_TRUE, 0, GL_READ_WRITE,
                       texture.meta.internal_format);
    glDispatchCompute(texture.meta.width, texture.meta.height,
                      texture.meta.depth);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT |
                    GL_TEXTURE_UPDATE_BARRIER_BIT);
  }

private:
  // utility function for checking shader compilation/linking errors.
  // ------------------------------------------------------------------------
  void checkCompileErrors(GLuint shader, std::string type, std::string path) {
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM") {
      glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
      if (!success) {
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        std::cout
            << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
            << path << "\n"
            << infoLog
            << "\n -- --------------------------------------------------- -- "
            << std::endl;
      }
    } else {
      glGetProgramiv(shader, GL_LINK_STATUS, &success);
      if (!success) {
        glGetProgramInfoLog(shader, 1024, NULL, infoLog);
        std::cout
            << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
            << path << "\n"
            << infoLog
            << "\n -- --------------------------------------------------- -- "
            << std::endl;
      }
    }
  }
};
}; // namespace ale::graphics
