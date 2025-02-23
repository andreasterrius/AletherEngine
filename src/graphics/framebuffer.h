//
// Created by Alether on 1/8/2025.
//

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "texture.h"
#include <external/assimp/code/AssetLib/Blender/BlenderDNA.h>
#include <memory>

namespace ale {

class FramebufferException final : public std::runtime_error {
public:
  explicit FramebufferException(const std::string &msg) : runtime_error(msg) {}
};

class Framebuffer {
public:
  enum ColorSpace {
    LINEAR, // always be in linear
    SRGB    // SRGB is for framebuffer that color attachment will be rendered to
            // the screen
  };
  struct Meta {
    int width = 0;
    int height = 0;
    ColorSpace color_space = SRGB;
  };

private:
  // temporary vars
  int start_frame_width = 0;
  int start_frame_height = 0;

  unsigned int framebuffer_id = 0;
  unsigned int depth_renderbuffer_id = 0;
  Meta meta;

  // this is the default color attachment
  std::shared_ptr<Texture> color_attachment0;
  std::vector<std::shared_ptr<Texture>> color_attachments;

public:
  Framebuffer(Meta meta);

  std::shared_ptr<Texture> recreate_color_attachment0();
  void create_extra_color_attachment(std::shared_ptr<Texture> attachment);
  void set_draw_buffers(const std::vector<unsigned int> &draw_buffers);

  void start_capture();

  void end_capture();

  std::shared_ptr<Texture> get_color_attachment0();

  glm::ivec2 get_size();

public:
  ~Framebuffer();

  Framebuffer(const Framebuffer &other) = delete;
  Framebuffer &operator=(Framebuffer &other) = delete;

  Framebuffer(Framebuffer &&other) noexcept;
  Framebuffer &operator=(Framebuffer &&other) noexcept;
};
}; // namespace ale

#endif // FRAMEBUFFER_H
