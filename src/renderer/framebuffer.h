//
// Created by Alether on 1/8/2025.
//

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "../texture.h"
#include <memory>

using namespace std;

namespace ale {

class FramebufferException final : public runtime_error {
public:
  explicit FramebufferException(const string &msg) : runtime_error(msg) {}
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
  shared_ptr<Texture> color_attachment0;

public:
  Framebuffer(Meta meta);

  shared_ptr<Texture> create_new_color_attachment0();

  void start_capture();

  void end_capture();

  shared_ptr<Texture> get_color_attachment0();

  ivec2 get_size();

public:
  ~Framebuffer();

  Framebuffer(const Framebuffer &other) = delete;
  Framebuffer &operator=(Framebuffer &other) = delete;

  Framebuffer(Framebuffer &&other) noexcept;
  Framebuffer &operator=(Framebuffer &&other) noexcept;
};
}; // namespace ale

#endif // FRAMEBUFFER_H
