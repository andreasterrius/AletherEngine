//
// Created by Alether on 10/14/2024.
//
module;

#include <fstream>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <unordered_map>

export module sdf_generator_gpu;
import compute_shader;
import texture;
import file_system;
import transform;
import mesh;
import bounding_box;

using afs = ale::FileSystem;
using namespace glm;
using namespace std;
using namespace ale::data;

export namespace ale::graphics::sdf {
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

  SdfGeneratorGPU() :
      compute_shader(afs::root("resources/shaders/sdf/sdf_generator_gpu.cs")) {
    // test
  }
  ~SdfGeneratorGPU() {
    for (auto &[k, v]: this->sdf_infos) {
      glDeleteBuffers(1, &v.index_ssbo);
      glDeleteBuffers(1, &v.vertex_ssbo);
      glDeleteBuffers(1, &v.bb_ubo);
    }
  }

  void add_mesh(string name, Mesh &mesh, int width, int height, int depth) {
    unsigned int vertices_size = mesh.vertices.size();
    unsigned int indices_size = mesh.indices.size();

    Data sdf_info;
    glGenBuffers(1, &sdf_info.vertex_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sdf_info.vertex_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 4 * sizeof(unsigned int) +
                     mesh.vertices.size() * sizeof(Vertex),
                 nullptr, GL_STATIC_DRAW);

    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 4 * sizeof(unsigned int),
                    &vertices_size); // pass size
    glBufferSubData(GL_SHADER_STORAGE_BUFFER,
                    4 * sizeof(unsigned int), // account for padding
                    mesh.vertices.size() * sizeof(Vertex),
                    mesh.vertices.data());

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

    BoundingBox outer_bb = mesh.boundingBox.apply_scale(Transform{
        .scale = vec3(1.1, 1.1, 1.1),
    });
    GpuBoundingBox gpu_bb{
        .inner_bb_min = vec4(mesh.boundingBox.min, 0.0),
        .inner_bb_max = vec4(mesh.boundingBox.max, 0.0),
        .outer_bb_min = vec4(outer_bb.min, 0.0),
        .outer_bb_max = vec4(outer_bb.max, 0.0),
    };
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GpuBoundingBox), &gpu_bb,
                 GL_STATIC_DRAW);

    // cout << "BB: " << gpu_bb.inner_bb_min.x << " " << gpu_bb.inner_bb_min.y
    // << " "
    //      << gpu_bb.inner_bb_min.z;
    // cout << " | " << gpu_bb.inner_bb_max.x << " " << gpu_bb.inner_bb_max.y <<
    // "
    // "
    //      << gpu_bb.inner_bb_max.z << endl;
    // cout << "BB: " << gpu_bb.outer_bb_min.x << " " << gpu_bb.outer_bb_min.y
    // << " "
    //      << gpu_bb.outer_bb_min.z;
    // cout << " | " << gpu_bb.outer_bb_max.x << " " << gpu_bb.outer_bb_max.y <<
    // "
    // "
    //      << gpu_bb.outer_bb_max.z << endl;

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

  void generate_all() {
    for (auto &[k, v]: this->sdf_infos) {
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

  Texture3D &at(string name) { return this->result.at(name); }

  void dump_textfile(string name, string filename = "") {
    auto result3d = this->result.at(name).retrieve_data_from_gpu();
    auto debug_result = this->debug_result.at(name).retrieve_data_from_gpu();

    if (filename == "")
      filename = name;

    ofstream out_file(afs::root("debug/" + filename + ".txt"));

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

    ofstream debug_out_file(afs::root("debug/" + name + "_debug.txt"));
    if (debug_out_file.is_open()) {
      int ctr = 0;
      for (int i = 0; i < DEBUG_TEXTURE_WIDTH; ++i) {
        for (int j = 0; j < DEBUG_TEXTURE_HEIGHT; ++j) {
          debug_out_file << "(" << debug_result[ctr] << ","
                         << debug_result[ctr + 1] << ","
                         << debug_result[ctr + 2] << ","
                         << debug_result[ctr + 3] << ") ";
          ctr += 4;
        }
        debug_out_file << endl;
      }
    }
    debug_out_file.close();
  }

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
} // namespace ale::graphics::sdf
