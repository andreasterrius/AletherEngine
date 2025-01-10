//
// Created by Alether on 1/9/2025.
//

#include "scene_viewport.h"

#include <imgui.h>

namespace ale::ui {

SceneViewport::SceneViewport(ivec2 screen_size)
    : framebuffer(Framebuffer::Meta{screen_size.x, screen_size.y}) {}

void SceneViewport::start_frame() { framebuffer.start_frame(); }

void SceneViewport::end_frame() { framebuffer.end_frame(); }

void SceneViewport::draw() {

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

  ImGui::Begin("Scene Viewport");

  ImVec2 window_size = ImGui::GetContentRegionAvail();
  ImGui::Image((GLuint)framebuffer.get_color_attachment0()->id, window_size,
               ImVec2(0, 1), ImVec2(1, 0));

  ImVec2 pos_min = ImGui::GetItemRectMin();
  ImVec2 pos_max = ImGui::GetItemRectMax();
  ImVec2 size = ImVec2(pos_max.x - pos_min.x, pos_max.y - pos_min.y);

  last_pos = ivec2(pos_min.x, pos_min.y);
  last_size = ivec2(size.x, size.y);

  /*
  // this returns from the viewport itself (not from the rendered image)
  auto viewport = ImGui::GetWindowViewport();
  auto im_pos = viewport->Pos;
  auto im_size = viewport->Size;
  last_pos = ivec2(im_pos.x, im_pos.y);
  last_size = ivec2(im_size.x, im_size.y);
  */

  ImGui::PopStyleVar();
  ImGui::End();
}

// world_pos needs to be relative to topleft of the screen
ivec2 SceneViewport::convert_to_viewport_pos(ivec2 world_pos) {
  ivec2 local_pos = ivec2(world_pos.x - last_pos.x, world_pos.y - last_pos.y);
  // local_pos *= framebuffer.get_size() / last_size;

  if (local_pos.x >= 0 && local_pos.y >= 0 && local_pos.x <= last_size.x &&
      local_pos.y <= last_size.y) {
    return local_pos;
  }

  return local_pos;
}
ivec2 SceneViewport::convert_to_framebuffer_pos(ivec2 world_pos) {
  vec2 logical_pos = convert_to_logical_pos(world_pos);

  // convert to framebuffer pos
  vec2 framebuffer_pos = logical_pos * vec2(framebuffer.get_size());

  return ivec2(framebuffer_pos);
}
vec2 SceneViewport::convert_to_logical_pos(ivec2 world_pos) {
  ivec2 viewport_pos = convert_to_viewport_pos(world_pos);

  // convert to logical pos
  vec2 logical_pos = vec2(viewport_pos) / vec2(last_size);

  return logical_pos;
}
bool SceneViewport::is_cursor_inside(ivec2 world_pos) {
  vec2 l = convert_to_logical_pos(world_pos);
  if (l.x < 0 || l.x >= last_size.x || l.y < 0 || l.y >= last_size.y) {
    return false;
  }
  return true;
}

Ray SceneViewport::create_mouse_ray(ivec2 global_pos, mat4 proj_mat,
                                    mat4 view_mat) {
  vec2 logical_pos = convert_to_logical_pos(global_pos);
  vec4 rayStartNdc =
      vec4((logical_pos.x * 2) - 1, (logical_pos.y * 2) - 1, -1.0f, 1.0f);
  vec4 rayEndNdc =
      vec4((logical_pos.x * 2) - 1, (logical_pos.y * 2) - 1, 0.0f, 1.0f);

  // not sure why this has to be inverted.
  rayStartNdc.y *= -1;
  rayEndNdc.y *= -1;

  mat4 invViewProj = inverse(proj_mat * view_mat);
  vec4 rayStartWorld = invViewProj * rayStartNdc;
  vec4 rayEndWorld = invViewProj * rayEndNdc;

  rayStartWorld /= rayStartWorld.w;
  rayEndWorld /= rayEndWorld.w;

  Ray r(rayStartWorld, normalize(rayEndWorld - rayStartWorld));

  return r;
}

} // namespace ale::ui
