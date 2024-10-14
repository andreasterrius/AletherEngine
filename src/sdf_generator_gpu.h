//
// Created by Alether on 10/14/2024.
//

#ifndef SDFGENERATOR_H
#define SDFGENERATOR_H
#include "data/compute_shader.h"
#include "data/mesh.h"
#include <unordered_map>
#include <string>

namespace ale {
class SDFGeneratorGPU {
public:
    struct Data {
        unsigned int vertex_ssbo = 0;
        unsigned int index_ssbo = 0;
        bool has_generated = false;
    };

private:
    ComputeShader compute_shader;

    unordered_map<string, Data> sdf_infos;
    unordered_map<string, Texture3D> result;
public:
    SDFGeneratorGPU();
    ~SDFGeneratorGPU();

    void add(string name, Mesh &mesh, int width, int height, int depth);

    void generate();
};
}


#endif //SDFGENERATOR_H
