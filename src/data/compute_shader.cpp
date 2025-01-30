//
// Created by Alether on 7/28/2024.
//
#include "compute_shader.h"
#include "../texture.h"
#include <fstream>
#include <iostream>
#include <sstream>

ale::ComputeShader::ComputeShader(string path) {
  std::ifstream computeShaderFile;
  string computeShaderCode =
      Shader::load_file_with_include(Shader::IncludeDirective{
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

  // if (textureDepth == 0){
  //     // we use 2d texture
  //     glGenTextures(1, &textureId);
  //     glActiveTexture(GL_TEXTURE0);
  //     glBindTexture(GL_TEXTURE_2D, textureId);
  //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  //     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, textureWidth, textureHeight,
  //     0, GL_RGBA,
  //                  GL_FLOAT, NULL);
  //
  //     glBindImageTexture(0, textureId, 0, GL_FALSE, 0, GL_READ_WRITE,
  //     GL_RGBA32F);
  // }
  // else{
  //     // we use 3d texture
  //     glGenTextures(1, &textureId);
  //     glActiveTexture(GL_TEXTURE0);
  //     glBindTexture(GL_TEXTURE_3D, textureId);
  //
  //     glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  //     glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  //     glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  //     glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  //     glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  //     glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, textureWidth, textureHeight,
  //     textureDepth,
  //         0, GL_RGBA, GL_FLOAT, NULL);
  //
  //     glBindImageTexture(0, textureId, 0, GL_FALSE, 0, GL_READ_WRITE,
  //     GL_RGBA32F);
  // }
}

void ale::ComputeShader::checkCompileErrors(GLuint shader, std::string type,
                                            std::string path) {
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

void ale::ComputeShader::execute_2d_save_to_texture_2d(Texture &texture) {
  glUseProgram(this->id);
  // TODO: this does not need to be bound inside hot loop actually. but
  // convinent
  glBindImageTexture(0, texture.id, 0, GL_FALSE, 0, GL_READ_WRITE,
                     texture.meta.internal_format);
  glDispatchCompute(texture.meta.width, texture.meta.height, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT |
                  GL_TEXTURE_UPDATE_BARRIER_BIT);
}

void ale::ComputeShader::execute_3d_save_to_texture_3d(Texture3D &texture) {
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

ale::ComputeShader::ComputeShader(ale::ComputeShader &&other) : id(other.id) {
  other.id = 0;
}

ale::ComputeShader &ale::ComputeShader::operator=(ale::ComputeShader &&other) {
  if (this != &other) {
    swap(this->id, other.id);
  }

  return *this;
}

ale::ComputeShader::~ComputeShader() { glDeleteProgram(id); }
