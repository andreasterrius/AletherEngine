//
// Created by Alether on 7/28/2024.
//
#include "compute_shader.h"
#include "../texture.h"
#include <iostream>
#include <fstream>
#include <sstream>

ale::ComputeShader::ComputeShader(string path) {
    std::ifstream computeShaderFile;
    string computeShaderCode;
    try{
        // open files
        computeShaderFile.open(path);
        std::stringstream shaderStream;
        // read file's buffer contents into streams
        shaderStream << computeShaderFile.rdbuf();
        // close file handlers
        computeShaderFile.close();
        // convert stream into string
        computeShaderCode = shaderStream.str();
        // if geometry shader path is present, also load a geometry shader
    }
    catch (std::ifstream::failure& e){
        std::cout << "ERROR::COMPUTE_SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        throw;
    }

    unsigned int compute = glCreateShader(GL_COMPUTE_SHADER);
    const char* cShaderCode = computeShaderCode.c_str();
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
    //     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, textureWidth, textureHeight, 0, GL_RGBA,
    //                  GL_FLOAT, NULL);
    //
    //     glBindImageTexture(0, textureId, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
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
    //     glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, textureWidth, textureHeight, textureDepth,
    //         0, GL_RGBA, GL_FLOAT, NULL);
    //
    //     glBindImageTexture(0, textureId, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    // }
}

void ale::ComputeShader::checkCompileErrors(GLuint shader, std::string type, std::string path) {
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM"){
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success){
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << path << "\n" << infoLog <<
                "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else{
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success){
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << path << "\n" << infoLog <<
                "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}

void ale::ComputeShader::executeAndSaveToTexture(Texture& texture) {
    glUseProgram(this->id);
    //TODO: this does not need to be bound inside hot loop actually. but convinent
    glBindImageTexture(0, texture.id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glDispatchCompute(texture.meta.width, texture.meta.height, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void ale::ComputeShader::executeAndSaveToTexture3D(Texture3D& texture) {
    glUseProgram(this->id);
    //TODO: this does not need to be bound inside hot loop actually. but convinent
    glBindImageTexture(0, texture.id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glDispatchCompute(texture.meta.width, texture.meta.height, texture.meta.depth);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}
