//
// Created by Alether on 10/14/2024.
//

#include "sdf_generator_gpu.h"
#include "data/mesh.h"
#include "file_system.h"
#include "sdf_model.h"
#include <filesystem>
#include <fstream>

using afs = ale::FileSystem;

ale::SDFGeneratorGPU::SDFGeneratorGPU()
    : compute_shader(afs::root("src/shaders/sdf_generator_gpu.cs")) {
  // test
}

SDFGeneratorGPU::~SDFGeneratorGPU() {
  for (auto &[k, v] : this->sdf_infos) {
    glDeleteBuffers(1, &v.index_ssbo);
    glDeleteBuffers(1, &v.vertex_ssbo);
    glDeleteBuffers(1, &v.bb_ubo);
  }

  for (auto &[k, v] : this->debug_result) {
  }

  for (auto &[k, v] : this->result) {
  }
}

void SDFGeneratorGPU::add_mesh(string name, Mesh &mesh, int width, int height,
                               int depth) {
  unsigned int vertices_size = mesh.vertices.size();
  unsigned int indices_size = mesh.indices.size();

  Data sdf_info;
  glGenBuffers(1, &sdf_info.vertex_ssbo);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, sdf_info.vertex_ssbo);
  glBufferData(GL_SHADER_STORAGE_BUFFER,
               4 * sizeof(unsigned int) + mesh.vertices.size() * sizeof(Vertex),
               nullptr, GL_STATIC_DRAW);

  glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 4 * sizeof(unsigned int),
                  &vertices_size); // pass size
  glBufferSubData(GL_SHADER_STORAGE_BUFFER,
                  4 * sizeof(unsigned int), // account for padding
                  mesh.vertices.size() * sizeof(Vertex), mesh.vertices.data());

  glGenBuffers(1, &sdf_info.index_ssbo);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, sdf_info.index_ssbo);
  glBufferData(GL_SHADER_STORAGE_BUFFER,
               sizeof(unsigned int) +
                   mesh.indices.size() * sizeof(unsigned int),
               nullptr, GL_STATIC_DRAW);

  glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int),
                  &indices_size); // pass size
  glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int),
                  mesh.indices.size() * sizeof(unsigned int),
                  mesh.indices.data());

  glGenBuffers(1, &sdf_info.bb_ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, sdf_info.bb_ubo);

  BoundingBox outer_bb = mesh.boundingBox.applyTransform(Transform{
      .scale = vec3(1.1, 1.1, 1.1),
  });
  GPUBoundingBox gpu_bb{
      .inner_bb_min = vec4(mesh.boundingBox.min, 0.0),
      .inner_bb_max = vec4(mesh.boundingBox.max, 0.0),
      .outer_bb_min = vec4(outer_bb.min, 0.0),
      .outer_bb_max = vec4(outer_bb.max, 0.0),
  };
  glBufferData(GL_UNIFORM_BUFFER, sizeof(GPUBoundingBox), &gpu_bb,
               GL_STATIC_DRAW);

  sdf_info.depth = depth;
  sdf_info.height = height;
  sdf_info.width = width;

  std::vector<float> empty;
  Texture3D texture = Texture3D(Texture3D::Meta{.width = width,
                                                .height = height,
                                                .depth = depth,
                                                .internal_format = GL_R32F,
                                                .input_format = GL_RED,
                                                .input_type = GL_FLOAT},
                                empty);

  this->sdf_infos.emplace(name, sdf_info);
  this->result.emplace(name, std::move(texture));
  this->debug_result.emplace(
      name, Texture(Texture::Meta{.width = DEBUG_TEXTURE_WIDTH,
                                  .height = DEBUG_TEXTURE_HEIGHT,
                                  .internal_format = GL_RGBA32F,
                                  .input_format = GL_RGBA,
                                  .input_type = GL_FLOAT},
                    empty));
}

void SDFGeneratorGPU::generate_all() {
  for (auto &[k, v] : this->sdf_infos) {
    if (!v.has_generated) {
      // base:0 IS bound inside the compute_shader
      glBindImageTexture(1, this->debug_result.at(k).id, 0, GL_FALSE, 0,
                         GL_READ_WRITE, GL_RGBA32F);
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, v.vertex_ssbo);
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, v.index_ssbo);
      glBindBufferBase(GL_UNIFORM_BUFFER, 4, v.bb_ubo);

      this->compute_shader.execute_3d_save_to_texture_3d(this->result.at(k));
      v.has_generated = true;
    }
  }
}

Texture3D &ale::SDFGeneratorGPU::at(string name) {
  return this->result.at(name);
}

void ale::SDFGeneratorGPU::dump_textfile(string name, string filename) {
  auto result3d = this->result.at(name).retrieve_data_from_gpu();
  auto debug_result = this->debug_result.at(name).retrieve_data_from_gpu();

  if (filename == "")
    filename = name;

  ofstream out_file(afs::root("resources/sdfgen/" + filename + ".txt"));

  auto sdf_info = sdf_infos.at(name);

  if (out_file.is_open()) {
    int ctr = 0;
    for (int i = 0; i < sdf_info.width; ++i) {
      out_file << "i: " << i << endl;
      for (int j = 0; j < sdf_info.height; ++j) {
        for (int k = 0; k < sdf_info.depth; ++k) {
          out_file << result3d[ctr] << " ";
          ctr++;
        }
        out_file << "| j: " << j << endl;
      }
      out_file << endl;
    }
  }
  out_file.close();

  ofstream debug_out_file(afs::root("resources/" + name + "_debug.txt"));
  if (debug_out_file.is_open()) {
    int ctr = 0;
    for (int i = 0; i < DEBUG_TEXTURE_WIDTH; ++i) {
      for (int j = 0; j < DEBUG_TEXTURE_HEIGHT; ++j) {
        debug_out_file << "(" << debug_result[ctr] << ","
                       << debug_result[ctr + 1] << "," << debug_result[ctr + 2]
                       << "," << debug_result[ctr + 3] << ") ";
        ctr += 4;
      }
      debug_out_file << endl;
    }
  }
  debug_out_file.close();
}
