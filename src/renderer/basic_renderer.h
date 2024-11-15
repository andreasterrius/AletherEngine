#ifndef BASIC_RENDERER_H
#define BASIC_RENDERER_H

#include "../camera.h"
#include "../data/shader.h"
#include <string>

namespace ale
{
// A basic renderer
class BasicRenderer
{
  private:
    Shader basicShader;

  public:
    BasicRenderer();

    void render(Camera &camera);
};
} // namespace ale

#endif // BASIC_RENDERER_H
