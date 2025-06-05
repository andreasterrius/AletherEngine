//
// Created by Alether on 1/9/2025.
//

module;

#include <glm/glm.hpp>
#include <imgui.h>
#include "src/graphics/framebuffer.h"

export module scene_viewport;
import ray;

using namespace glm;

export namespace ale::editor {
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
  SceneViewport(ivec2 initial_screen_size) :
      framebuffer(Framebuffer::Meta{
          initial_screen_size.x, initial_screen_size.y, Framebuffer::LINEAR}) {}

  void start_capture() { framebuffer.start_capture(); }

  void end_capture() { framebuffer.end_capture(); }

  void draw() {

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    ImGui::Begin(panel_name.c_str(), nullptr, ImGuiWindowFlags_NoCollapse);

    ImVec2 window_size = ImGui::GetContentRegionAvail();
    ImGui::Image((GLuint) framebuffer.get_color_attachment0()->id, window_size,
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

  // returns <-1, -1> if the world_pos is outside the viewport window
  glm::ivec2 convert_to_viewport_pos(ivec2 world_pos) {
    ivec2 local_pos = ivec2(world_pos.x - last_pos.x, world_pos.y - last_pos.y);
    // local_pos *= framebuffer.get_size() / last_size;

    if (local_pos.x >= 0 && local_pos.y >= 0 && local_pos.x <= last_size.x &&
        local_pos.y <= last_size.y) {
      return local_pos;
    }

    return local_pos;
  }

  glm::ivec2 convert_to_framebuffer_pos(ivec2 world_pos) {
    vec2 logical_pos = convert_to_logical_pos(world_pos);

    // convert to framebuffer pos
    vec2 framebuffer_pos = logical_pos * vec2(framebuffer.get_size());

    return ivec2(framebuffer_pos);
  }

  glm::vec2 convert_to_logical_pos(ivec2 world_pos) {
    ivec2 viewport_pos = convert_to_viewport_pos(world_pos);

    // convert to logical pos
    vec2 logical_pos = vec2(viewport_pos) / vec2(last_size);

    return logical_pos;
  }

  bool is_cursor_inside(ivec2 world_pos) {
    vec2 l = convert_to_logical_pos(world_pos);
    if (l.x < 0 || l.x >= 1.0 || l.y < 0 || l.y >= 1.0) {
      return false;
    }
    return true;
  }


  // only create ray if the mouse cursor is inside the viewport
  ale::graphics::Ray create_mouse_ray(ivec2 global_pos, mat4 proj_mat,
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

    graphics::Ray r(rayStartWorld, normalize(rayEndWorld - rayStartWorld));

    return r;
  }
};
} // namespace ale::editor
