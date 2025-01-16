//
// Created by Alether on 1/8/2025.
//

#include "framebuffer.h"

#include <spdlog/spdlog.h>
ale::Framebuffer::Framebuffer(Meta meta) : meta(meta) {
  SPDLOG_TRACE("Creating framebuffer {} {}", meta.width, meta.height);
  glEnable(GL_FRAMEBUFFER_SRGB);
  glGenFramebuffers(1, &framebuffer_id);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);

  auto empty = vector<float>();
  this->color_attachment0 = make_shared<Texture>(
      Texture::Meta{
          .width = meta.width,
          .height = meta.height,
          .internal_format = GL_SRGB8_ALPHA8,
          .input_format = GL_RGBA,
          .input_type = GL_UNSIGNED_BYTE,
      },
      empty);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         color_attachment0->id, 0);

  glGenRenderbuffers(1, &depth_renderbuffer_id);
  glBindRenderbuffer(GL_RENDERBUFFER, depth_renderbuffer_id);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, meta.width,
                        meta.height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, depth_renderbuffer_id);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    throw FramebufferException("framebuffer is not complete");
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  SPDLOG_TRACE("Created framebuffer {} {}", meta.width, meta.height);
}

shared_ptr<Texture> ale::Framebuffer::create_new_color_attachment0() {
  auto before = this->color_attachment0;

  auto empty = vector<float>();
  this->color_attachment0 =
      make_shared<Texture>(this->color_attachment0->meta, empty);

  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         color_attachment0->id, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return before;
}

void ale::Framebuffer::start_frame() {
  int viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  start_frame_width = viewport[2];
  start_frame_height = viewport[3];
  glViewport(0, 0, this->meta.width, this->meta.height);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);
}

void ale::Framebuffer::end_frame() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, start_frame_width, start_frame_height);
}

shared_ptr<Texture> ale::Framebuffer::get_color_attachment0() {
  return color_attachment0;
}

ivec2 ale::Framebuffer::get_size() { return ivec2(meta.width, meta.height); }

ale::Framebuffer::~Framebuffer() {
  glDeleteRenderbuffers(1, &depth_renderbuffer_id);
  glDeleteFramebuffers(1, &framebuffer_id);
}

ale::Framebuffer::Framebuffer(Framebuffer &&other) noexcept
    : framebuffer_id{other.framebuffer_id},
      depth_renderbuffer_id{other.depth_renderbuffer_id},
      color_attachment0{std::move(other.color_attachment0)}, meta(other.meta),
      start_frame_width(other.start_frame_width),
      start_frame_height(other.start_frame_height) {
  other.framebuffer_id = 0;
  other.depth_renderbuffer_id = 0;
}

ale::Framebuffer &ale::Framebuffer::operator=(Framebuffer &&other) noexcept {
  if (this != &other) {
    swap(this->color_attachment0, other.color_attachment0);
    swap(this->depth_renderbuffer_id, other.depth_renderbuffer_id);
    swap(this->meta, other.meta);
    swap(this->framebuffer_id, other.framebuffer_id);
    swap(this->start_frame_width, other.start_frame_width);
    swap(this->start_frame_height, other.start_frame_height);
  }

  return *this;
}