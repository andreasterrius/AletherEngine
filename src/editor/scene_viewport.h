//
// Created by Alether on 1/9/2025.
//

#ifndef SCENE_VIEWPORT_H
#define SCENE_VIEWPORT_H

#include "src/graphics/framebuffer.h"
#include "src/graphics/ray.h"
#include <imgui.h>

namespace ale::editor {
class SceneViewport {
private:
  Framebuffer framebuffer;
  // Last known pos and size during draw
  glm::ivec2 last_pos = glm::ivec2();
  glm::ivec2 last_size = glm::ivec2();

public:
  const std::string panel_name = "Scene Viewport";

  // TODO: do we render on screen size then downscale to viewport size?
  // TODO: or viewport size directly? I'm not sure.. choosing former for now
  // TODO: resize framebuffer when window is resized?
  SceneViewport(glm::ivec2 initial_screen_size);

  void start_capture();

  void end_capture();

  void draw();

  // returns <-1, -1> if the world_pos is outside the viewport window
  glm::ivec2 convert_to_viewport_pos(glm::ivec2 world_pos);

  glm::ivec2 convert_to_framebuffer_pos(glm::ivec2 world_pos);

  glm::vec2 convert_to_logical_pos(glm::ivec2 world_pos);

  bool is_cursor_inside(glm::ivec2 world_pos);

  // only create ray if the mouse cursor is inside the viewport
  Ray create_mouse_ray(glm::ivec2 global_pos, glm::mat4 proj_mat,
                       glm::mat4 view_mat);

  ImGuiID get_panel_id();
};
} // namespace ale::ui

#endif // SCENE_VIEWPORT_H
