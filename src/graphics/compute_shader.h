#ifndef COMPUTE_SHADER_H
#define COMPUTE_SHADER_H

#include "texture.h"
#include <glad/glad.h>
#include <string>

namespace ale {
class ComputeShader {
public:
  unsigned int id;

  ComputeShader(std::string path);
  ~ComputeShader();

  ComputeShader(const ComputeShader &other) = delete;
  ComputeShader &operator=(const ComputeShader &other) = delete;

  ComputeShader(ComputeShader &&other);
  ComputeShader &operator=(ComputeShader &&other);

  void execute_2d_save_to_texture_2d(Texture &texture);

  void execute_3d_save_to_texture_3d(Texture3D &texture);

private:
  // utility function for checking shader compilation/linking errors.
  // ------------------------------------------------------------------------
  void checkCompileErrors(GLuint shader, std::string type, std::string path);
};
}; // namespace ale
#endif // COMPUTE_SHADER_H
