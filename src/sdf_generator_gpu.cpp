//
// Created by Alether on 10/14/2024.
//

#include "sdf_generator_gpu.h"
#include "file_system.h"
#include "sdf_model.h"
#include "data/mesh.h"

using afs = ale::FileSystem;

ale::SDFGeneratorGPU::SDFGeneratorGPU()
    : compute_shader(afs::root("src/shaders/sdf_generator_gpu.comp")) {
    //test
}

SDFGeneratorGPU::~SDFGeneratorGPU() {
    for (auto& [k, v] : this->sdf_infos){
        glDeleteBuffers(1, &v.index_ssbo);
        glDeleteBuffers(1, &v.vertex_ssbo);
    }
}

void SDFGeneratorGPU::add(string name, Mesh& mesh, int width, int height, int depth) {
    Data sdf_info;
    glGenBuffers(1, &sdf_info.vertex_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sdf_info.vertex_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 mesh.vertices.size() * sizeof(Vertex),
                 mesh.vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &sdf_info.index_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sdf_info.index_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 mesh.indices.size() * sizeof(unsigned int),
                 mesh.indices.data(), GL_STATIC_DRAW);

    this->sdf_infos.emplace(name, sdf_info);
    this->result.emplace(name, Texture3D(Texture3D::Meta{
                             .width = width,
                             .height = height,
                             .depth = depth,
                             .internal_format = GL_RGBA32F,
                             .input_format = GL_RGBA,
                             .input_type = GL_FLOAT
                         }));
    this->debug_result.emplace(name, Texture(Texture::Meta{
                                   .width = width,
                                   .height = height,
                                   .internal_format = GL_RGBA32F,
                                   .input_format = GL_RGBA,
                                   .input_type = GL_FLOAT
                               }));
}

void SDFGeneratorGPU::generate() {
    for (auto& [k, v] : this->sdf_infos){
        if (!v.has_generated){
            glBindImageTexture(1, this->debug_result.at(k).id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, v.vertex_ssbo);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, v.index_ssbo);

            this->compute_shader.executeAndSaveToTexture3D(this->result.at(k));
            v.has_generated = true;
        }
    }
}

void SDFGeneratorGPU::dump(string path) {

}
