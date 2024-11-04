//
// Created by Alether on 10/14/2024.
//

#include "sdf_generator_gpu.h"
#include "file_system.h"
#include "sdf_model.h"
#include "data/mesh.h"
#include <fstream>

using afs = ale::FileSystem;

ale::SDFGeneratorGPU::SDFGeneratorGPU()
    : compute_shader(afs::root("src/shaders/sdf_generator_gpu.cs"))
{
    // test
}

SDFGeneratorGPU::~SDFGeneratorGPU()
{
    for (auto &[k, v] : this->sdf_infos)
    {
        glDeleteBuffers(1, &v.index_ssbo);
        glDeleteBuffers(1, &v.vertex_ssbo);
    }
}

void SDFGeneratorGPU::add(string name, Mesh &mesh, int width, int height, int depth)
{
    Data sdf_info;
    glGenBuffers(1, &sdf_info.vertex_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sdf_info.vertex_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 sizeof(unsigned int) + mesh.vertices.size() * sizeof(Vertex),
                 nullptr, GL_STATIC_DRAW);

    unsigned int vertices_size = mesh.vertices.size();
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), &vertices_size); // pass size
    glBufferSubData(GL_SHADER_STORAGE_BUFFER,
                    sizeof(unsigned int),
                    mesh.vertices.size() * sizeof(Vertex),
                    mesh.vertices.data());

    glGenBuffers(1, &sdf_info.index_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sdf_info.index_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 sizeof(unsigned int) + mesh.indices.size() * sizeof(unsigned int),
                 nullptr, GL_STATIC_DRAW);

    unsigned int indices_size = mesh.indices.size();
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), &indices_size); // pass size
    glBufferSubData(GL_SHADER_STORAGE_BUFFER,
                    sizeof(unsigned int),
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
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GPUBoundingBox), &gpu_bb, GL_STATIC_DRAW);

    sdf_info.depth = depth;
    sdf_info.height = height;
    sdf_info.width = width;

    this->sdf_infos.emplace(name, sdf_info);
    this->result.emplace(name, Texture3D(Texture3D::Meta{
                                   .width = width,
                                   .height = height,
                                   .depth = depth,
                                   .internal_format = GL_R32F,
                                   .input_format = GL_RED,
                                   .input_type = GL_FLOAT}));
    this->debug_result.emplace(name, Texture(Texture::Meta{
                                         .width = width,
                                         .height = height,
                                         .internal_format = GL_RGBA32F,
                                         .input_format = GL_RGBA,
                                         .input_type = GL_FLOAT}));
}

void SDFGeneratorGPU::generate()
{
    for (auto &[k, v] : this->sdf_infos)
    {
        if (!v.has_generated)
        {
            // base:0 IS bound inside the compute_shader
            glBindImageTexture(1, this->debug_result.at(k).id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, v.vertex_ssbo);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, v.index_ssbo);
            glBindBufferBase(GL_UNIFORM_BUFFER, 4, v.bb_ubo);

            this->compute_shader.executeAndSaveToTexture3D(this->result.at(k));
            v.has_generated = true;
        }
    }
}

void SDFGeneratorGPU::dump(string name)
{
    auto result3d = this->result.at(name).dump_data_from_gpu();

    ofstream out_file(afs::root("resources/" + name + ".bin"), std::ios::binary);
    out_file.write(reinterpret_cast<char *>(result3d.data()), result3d.size() * sizeof(float));
    out_file.close();
}

void ale::SDFGeneratorGPU::dump_textfile(string name)
{
    auto result3d = this->result.at(name).dump_data_from_gpu();

    ofstream out_file(afs::root("resources/" + name + ".txt"));
    // out_file.write(reinterpret_cast<char *>(result3d.data()), result3d.size() * sizeof(float));
    
    auto sdf_info = sdf_infos.at(name);

    if (out_file.is_open()){
        int ctr = 0;
        for (int i = 0; i < sdf_info.width; ++i){
            out_file << "i: " << i << endl;
            for (int j = 0; j < sdf_info.height; ++j){
                for (int k = 0; k < sdf_info.depth; ++k){
                    out_file << result3d[ctr] << " ";
                    ctr++;
                }
                out_file << "| j: " << j << endl;
            }
            out_file << endl;
        }
    }


    out_file.close();
}
