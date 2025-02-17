//
// Created by Alether on 1/27/2025.
//

#include "sdf_generator_gpu_v2.h"

#include "sdf_generator_gpu_v2_shared.h"
#include "src/data/file_system.h"

using afs = ale::FileSystem;
using namespace std;
using namespace glm;

namespace ale {
SdfGeneratorGPUV2::SdfGeneratorGPUV2()
    : sdfgen_v2(
          afs::root("resources_new/shaders/sdf/sdf_generator_gpu_v2.cs")) {}

vector<Texture3D> SdfGeneratorGPUV2::generate_cpu(Model &M, int resolution) {
  return {};
}

vector<Texture3D> SdfGeneratorGPUV2::generate_gpu(Model &m, int resolution) {

  auto sdfs = vector<Texture3D>();
  for (auto &mesh : m.meshes) {
    sdfs.emplace_back(std::move(generate_gpu(mesh, resolution)));
  }
  return sdfs;
}
Texture3D SdfGeneratorGPUV2::generate_gpu(Mesh &mesh, int resolution) {
  unsigned int vertex_buffer;
  unsigned int index_buffer;
  unsigned int ubo;

  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertex_buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, mesh.vertices.size() * sizeof(Vertex),
               mesh.vertices.data(), GL_STATIC_DRAW);

  glGenBuffers(1, &index_buffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, index_buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER,
               mesh.indices.size() * sizeof(unsigned int), mesh.indices.data(),
               GL_STATIC_DRAW);

  auto outer_bb = mesh.boundingBox.apply_scale(Transform{
      .scale = vec3(1.1, 1.1, 1.1),
  });
  auto gpu_data =
      GpuData{ivec4(mesh.vertices.size(), mesh.indices.size(), 0, 0),
              vec4(mesh.boundingBox.min, 0.0), vec4(mesh.boundingBox.max, 0.0),
              vec4(outer_bb.min, 0.0), vec4(outer_bb.max, 0.0)};

  glGenBuffers(1, &ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, ubo);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(GpuData), &gpu_data, GL_STATIC_DRAW);

  // wait until the upload is done
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

  std::vector<float> empty;
  Texture3D texture = Texture3D(Texture3D::Meta{.width = resolution,
                                                .height = resolution,
                                                .depth = resolution,
                                                .internal_format = GL_R32F,
                                                .input_format = GL_RED,
                                                .input_type = GL_FLOAT},
                                empty);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, vertex_buffer);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, index_buffer);
  glBindBufferBase(GL_UNIFORM_BUFFER, 4, ubo);

  this->sdfgen_v2.execute_3d_save_to_texture_3d(texture);

  glDeleteBuffers(1, &vertex_buffer);
  glDeleteBuffers(1, &index_buffer);
  glDeleteBuffers(1, &ubo);

  return texture;
}

} // namespace ale
