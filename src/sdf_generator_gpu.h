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
    const int DEBUG_TEXTURE_WIDTH = 1024;
    const int DEBUG_TEXTURE_HEIGHT = 1;

    class SDFGeneratorGPU {
    public:
        
        // Passed as UBO
        struct GPUBoundingBox
        {
            vec4 inner_bb_min;
            vec4 inner_bb_max;
            vec4 outer_bb_min;
            vec4 outer_bb_max;
        } ;

        struct Data {
            unsigned int vertex_ssbo = 0;
            unsigned int index_ssbo = 0;
            unsigned int bb_ubo = 0;
            bool has_generated = false;
            int width;
            int height;
            int depth;
        };

    private:
        ComputeShader compute_shader;

        unordered_map<string, Data> sdf_infos;

    public:
        unordered_map<string, Texture3D> result;
        unordered_map<string, Texture> debug_result;

        SDFGeneratorGPU();
        ~SDFGeneratorGPU();

        void add_mesh(string name, Mesh& mesh, int width, int height, int depth);

        void generate_all();

        Texture3D &at(string name);

        void dump_textfile(string name);
    };
}


#endif //SDFGENERATOR_H
