//
// Created by Alether on 1/9/2025.
//

#ifndef SCENE_VIEWPORT_H
#define SCENE_VIEWPORT_H
#include "../framebuffer.h"
#include "src/data/ray.h"
#include <imgui.h>

namespace ale::ui {
class SceneViewport {
private:
  Framebuffer framebuffer;
  // Last known pos and size during draw
  ivec2 last_pos = ivec2();
  ivec2 last_size = ivec2();

public:
  const string panel_name = "Scene Viewport";

  // TODO: do we render on screen size then downscale to viewport size?
  // TODO: or viewport size directly? I'm not sure.. choosing former for now
  // TODO: resize framebuffer when window is resized?
  SceneViewport(ivec2 initial_screen_size);

  void start_frame();

  void end_frame();

  void draw();

  // returns <-1, -1> if the world_pos is outside the viewport window
  ivec2 convert_to_viewport_pos(ivec2 world_pos);

  ivec2 convert_to_framebuffer_pos(ivec2 world_pos);

  vec2 convert_to_logical_pos(ivec2 world_pos);

  bool is_cursor_inside(ivec2 world_pos);

  // only create ray if the mouse cursor is inside the viewport
  Ray create_mouse_ray(ivec2 global_pos, mat4 proj_mat, mat4 view_mat);

  ImGuiID get_panel_id();
};
} // namespace ale::ui

#endif // SCENE_VIEWPORT_H
