#include "basic_renderer.h"
#include "../file_system.h"

using afs = ale::FileSystem;
using namespace ale;

BasicRenderer::BasicRenderer()
    : basic_shader(afs::root("src/renderer/basic_renderer.vs").c_str(),
                   afs::root("src/renderer/basic_renderer.fs").c_str())
{
}

void ale::BasicRenderer::render(Camera &camera, vector<Light> &lights, vector<Renderable> &renderables)
{
    // TODO: the usual render stuff
}

void ale::BasicRenderer::prepare_shadowable_objects(vector<Renderable> &renderables)
{
    SDFGeneratorGPU sdfgen;

    // In this function, we will change 3d texture in 2d texture atlas
    // The reason being, it's harder to batch 3d textures compared to 2d textures.
    for (auto &renderable : renderables)
    {
        if (renderable.shadow.has_value())
        {
            for (int i = 0; i < renderable.model->meshes.size(); ++i)
            {
                int r = renderable.shadow->resolution;
                // TODO: check if existing sdf already exist, no need to re-generate
                sdfgen.add_mesh(renderable.model->path.filename().string() + "-" + to_string(i) + "-" + to_string(r),
                                renderable.model->meshes[i], r, r, r);
            }
        }
    }

    sdfgen.generate_all();

    auto flat_data = vector<float>(4096 * 4096, 0.0f);
    int index = -1;

    for (auto it = sdfgen.begin(); it != sdfgen.end(); ++it)
    {
        auto sdf_data = it->second.retrieve_data_from_gpu();
        Texture3D::Meta meta = it->second.meta;
        sdfgen.dump_textfile(it->first);

        index += 1;
        ivec3 size = ivec3(meta.width, meta.height, meta.depth);
        for (int i = 0; i < sdf_data.size(); ++i)
        {
            unsigned int z = i / (size.x * size.y);
            unsigned int y = (i % (size.x * size.y) / size.x);
            unsigned int x = i % size.x;

            unsigned int flat_x = x * z * size.x;
            unsigned int flat_y = y;
            unsigned int flat_index = x + (index * 64) + (y * size.x);

            // finish the remapping
            flat_data[flat_index] = sdf_data[i];
        }

        if (std::next(it) == sdfgen.end() || index >= 64)
        {
            // we has filled in this texture, push and create a new one
            texture_atlas.emplace_back(
                Texture(Texture::Meta{
                            .width = 4096,
                            .height = 256,
                            .internal_format = GL_R32F,
                            .input_format = GL_RED,
                            .input_type = GL_FLOAT},
                        flat_data),
                index);
            index = -1;
        }
    }

    if (debug_mode)
    {
        for (int i = 0; i < texture_atlas.size(); ++i)
        {
            texture_atlas[i].texture.dump_data_to_file(afs::root("resources/sdfgen/atlas_" + to_string(i) + ".txt"));
        }
    }
}
