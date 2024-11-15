#include "basic_renderer.h"
#include "../file_system.h"

using afs = ale::FileSystem;
using namespace ale;

BasicRenderer::BasicRenderer()
    : basicShader(afs::root("src/renderer/basic_renderer.vs").c_str(),
                  afs::root("src/renderer/basic_renderer.fs").c_str()) {}

void ale::BasicRenderer::render(Camera &camera, vector<Light> &lights, vector<Renderable> &renderables)
{
    //TODO: the usual render stuff
}

void ale::BasicRenderer::prepare_shadowable_objects(vector<Renderable> &renderables)
{
    //TODO: prepare the texture atlas or use texture array
}

