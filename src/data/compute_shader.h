//
// Created by Alether on 7/28/2024.
//

#ifndef COMPUTE_SHADER_H
#define COMPUTE_SHADER_H

#include<string>
#include <glad/glad.h>

using namespace std;

namespace ale {
class ComputeShader {
public:
    unsigned int id;
    unsigned int textureWidth, textureHeight;
    unsigned int textureId;

    unsigned int ssboId;

    ComputeShader(string path, int textureWidth, int textureHeight);

    void execute();

private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(GLuint shader, std::string type, std::string path);

};
}

#endif //COMPUTE_SHADER_H
