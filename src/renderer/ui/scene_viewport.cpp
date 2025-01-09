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
  ImGui::Begin("Scene Viewport");

  ImVec2 window_size = ImGui::GetContentRegionAvail();
  ImGui::Image((GLuint)framebuffer.get_color_attachment0()->id, window_size,
               ImVec2(0, 1), ImVec2(1, 0));

  auto im_pos = ImGui::GetWindowPos();
  auto im_size = ImGui::GetWindowSize();
  last_pos = ivec2(im_pos.x, im_pos.y);
  last_size = ivec2(im_size.x, im_size.y);

  ImGui::End();
}
} // namespace ale::ui
