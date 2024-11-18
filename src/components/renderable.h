#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "../data/model.h"
#include "../data/transform.h"
#include <memory>
#include <optional>
#include <vector>

namespace ale {
struct SDFShadowMeta {
  int resolution;
};

struct Renderable {
  Transform transform; // TODO: is it supposed to be here?
  std::optional<SDFShadowMeta> shadow;
  std::shared_ptr<Model> model;
};
} // namespace ale

#endif
