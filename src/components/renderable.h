#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "../data/model.h"
#include "../data/transform.h"
#include <optional>
#include <memory>
#include <vector>

namespace ale
{
    class SDFShadowMeta
    {
        int resolution;
    };

    class Renderable
    {
        Transform transform; //TODO: is it supposed to be here?
        std::optional<SDFShadowMeta> shadow;
        std::shared_ptr<Model> model;
    };
}

#endif