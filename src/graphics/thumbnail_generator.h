//
// Created by Alether on 1/8/2025.
//

#ifndef THUMBNAIL_GENERATOR_H
#define THUMBNAIL_GENERATOR_H

#include "../data/static_mesh.h"
#include "basic_renderer.h"
#include "framebuffer.h"

constexpr int DEFAULT_THUMBNAIL_WIDTH = 400;
constexpr int DEFAULT_THUMBNAIL_HEIGHT = 400;

namespace ale {
class ThumbnailGenerator {
  BasicRenderer renderer;
  Framebuffer framebuffer;

public:
  explicit ThumbnailGenerator(int thumbnail_width = DEFAULT_THUMBNAIL_WIDTH,
                              int thumbnail_height = DEFAULT_THUMBNAIL_HEIGHT);

  shared_ptr<Texture> generate(StaticMesh static_mesh);
};
} // namespace ale

#endif // THUMBNAIL_GENERATOR_H
