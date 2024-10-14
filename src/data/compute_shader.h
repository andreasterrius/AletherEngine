#ifndef COMPUTE_SHADER_H
#define COMPUTE_SHADER_H

#include<string>
#include <glad/glad.h>
#include "src/texture.h"

using namespace std;

namespace ale {
class ComputeShader {
public:
    unsigned int id;

    ComputeShader(string path);

    void executeAndSaveToTexture(Texture& texture);

    void executeAndSaveToTexture3D(Texture3D& texture);

private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(GLuint shader, std::string type, std::string path);
};
};
#endif //COMPUTE_SHADER_H
