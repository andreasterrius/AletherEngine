//
// Created by Alether on 1/27/2025.
//

#ifndef SDF_GENERATOR_GPU_V2_H
#define SDF_GENERATOR_GPU_V2_H

#include "../data/model.h"
#include "compute_shader.h"

namespace ale {
class SdfGeneratorGPUV2 {
  ComputeShader sdfgen_v2;

  struct GpuData {
    glm::ivec4 size; // [0] = vertices.size, [1] = indices.size, [2] and [3] is
                     // unused (0)
    glm::vec4 inner_bb_min;
    glm::vec4 inner_bb_max;
    glm::vec4 outer_bb_min;
    glm::vec4 outer_bb_max;
  };

  struct Buffers {
    unsigned int vertex_buffer;
    unsigned int index_buffer;
    unsigned int ubo;
  };

public:
  SdfGeneratorGPUV2();

  vector<Texture3D> generate_cpu(Model &M, int resolution);
  vector<Texture3D> generate_gpu(Model &m, int resolution);
  Texture3D generate_gpu(Mesh &mesh, int resolution);
};

} // namespace ale

#endif // SDF_GENERATOR_GPU_V2_H
