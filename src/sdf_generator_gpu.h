//
// Created by Alether on 10/14/2024.
//

#ifndef SDFGENERATOR_H
#define SDFGENERATOR_H
#include "data/mesh.h"
#include "graphics/compute_shader.h"
#include <string>
#include <unordered_map>

namespace ale {
const int DEBUG_TEXTURE_WIDTH = 1024;
const int DEBUG_TEXTURE_HEIGHT = 1;

class SdfGeneratorGPU {
public:
  // Passed as UBO
  struct GpuBoundingBox {
    glm::vec4 inner_bb_min;
    glm::vec4 inner_bb_max;
    glm::vec4 outer_bb_min;
    glm::vec4 outer_bb_max;
  };

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

  std::unordered_map<std::string, Data> sdf_infos;

public:
  std::unordered_map<std::string, Texture3D> result;

  std::unordered_map<std::string, Texture> debug_result;

  SdfGeneratorGPU();
  ~SdfGeneratorGPU();

  void add_mesh(std::string name, Mesh &mesh, int width, int height, int depth);

  void generate_all();

  Texture3D &at(std::string name);

  void
  dump_textfile(std::string name,
                std::string filename = "" /*if not provided namne == path*/);

public:
  // Iterator
  using ResultIterator = std::unordered_map<std::string, Texture3D>::iterator;
  using ConstResultIterator =
      std::unordered_map<std::string, Texture3D>::const_iterator;

  ResultIterator begin() { return result.begin(); }
  ResultIterator end() { return result.end(); }
  ConstResultIterator cbegin() const { return result.cbegin(); }
  ConstResultIterator cend() const { return result.cend(); }
};
} // namespace ale

#endif // SDFGENERATOR_H
