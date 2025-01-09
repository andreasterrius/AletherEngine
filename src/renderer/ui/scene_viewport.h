//
// Created by Alether on 1/9/2025.
//

#ifndef SCENE_VIEWPORT_H
#define SCENE_VIEWPORT_H
#include "../framebuffer.h"

namespace ale::ui {
class SceneViewport {
private:
  Framebuffer framebuffer;
  // Last known pos and size during draw
  ivec2 last_pos = ivec2();
  ivec2 last_size = ivec2();

public:
  // TODO: do we render on screen size then downscale to viewport size?
  // TODO: or viewport size directly? I'm not sure.. choosing former for now
  // TODO: resize framebuffer when window is resized?
  SceneViewport(ivec2 screen_size);

  void start_frame();

  void end_frame();

  void draw();
};
} // namespace ale::ui

#endif // SCENE_VIEWPORT_H
