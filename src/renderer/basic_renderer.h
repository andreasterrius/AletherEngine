#ifndef BASIC_RENDERER_H
#define BASIC_RENDERER_H

#include "../camera.h"
#include "../data/shader.h"
#include "../components/renderable.h"
#include <string>

namespace ale
{
    struct Light
    {
    };

    class BasicRenderer
    {
    private:
        Shader basicShader;

    public:
        BasicRenderer();

        void render(Camera &camera, vector<Light> &lights, vector<Renderable> &renderables);

        void prepare_shadowable_objects(vector<Renderable> &renderables);
    };
} // namespace ale

#endif // BASIC_RENDERER_H
