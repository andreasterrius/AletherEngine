#ifndef BASIC_RENDERER_H
#define BASIC_RENDERER_H

#include "../camera.h"
#include "../components/renderable.h"
#include "../data/shader.h"
#include "../sdf_generator_gpu.h"
#include <string>

namespace ale {
struct Light {};

// Basic renderer packs 3d texture into 2d texture
// but we still need to keep some offsets and pointers for it
struct TextureAtlas {
  Texture texture;
  int size;
};

class BasicRenderer {
private:
  Shader basic_shader;

  vector<TextureAtlas> texture_atlas;

  bool debug_mode = true;

public:
  BasicRenderer();

  void render(Camera &camera, vector<Light> &lights,
              vector<Renderable> &renderables);

  // TODO: I'm pretty sure this will be refactored out somewhere
  void prepare_shadowable_objects(vector<Renderable> &renderables);
};
} // namespace ale

#endif // BASIC_RENDERER_H
