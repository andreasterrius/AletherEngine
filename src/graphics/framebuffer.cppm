//
// Created by Alether on 1/8/2025.
//

module;

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <variant>
#include <vector>

export module graphics:framebuffer;
import :texture;

using namespace std;
using namespace glm;

export namespace ale::graphics {

class FramebufferException final : public std::runtime_error {
public:
  explicit FramebufferException(const std::string &msg) : runtime_error(msg) {}
};

class Framebuffer {
public:
  enum ColorSpace {
    LINEAR, // always be in linear
    SRGB // SRGB is for framebuffer that color attachment will be rendered to
         // the screen
  };
  struct Meta {
    int width = 0;
    int height = 0;
    ColorSpace color_space = SRGB;
    bool depthbuffer_texture = false;
  };

private:
  // temporary vars
  int start_frame_width = 0;
  int start_frame_height = 0;

  unsigned int framebuffer_id = 0;
  // unsigned int depth_renderbuffer_id = 0;
  Meta meta;

  // depth attachment (texture or rbo)
  std::variant<unsigned int, std::shared_ptr<Texture>> depth_buffer;

  // this is the default color attachment
  std::shared_ptr<Texture> color_attachment0;
  std::vector<std::shared_ptr<Texture>> color_attachments; // extra attachments

public:
  Framebuffer(Meta meta) : meta(meta) {
    SPDLOG_TRACE("Creating framebuffer {} {}", meta.width, meta.height);
    // glEnable(GL_FRAMEBUFFER_SRGB);
    glGenFramebuffers(1, &framebuffer_id);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);

    auto empty = vector<float>();

    if (meta.color_space == LINEAR) {
      this->color_attachment0 = make_shared<Texture>(
          Texture::Meta{
              .width = meta.width,
              .height = meta.height,
              .internal_format = GL_RGBA8,
              .input_format = GL_RGBA,
              .input_type = GL_UNSIGNED_BYTE,
          },
          empty);
    } else {
      this->color_attachment0 = make_shared<Texture>(
          Texture::Meta{
              .width = meta.width,
              .height = meta.height,
              .internal_format = GL_SRGB8_ALPHA8,
              .input_format = GL_RGBA,
              .input_type = GL_UNSIGNED_BYTE,
          },
          empty);
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           color_attachment0->id, 0);

    if (meta.depthbuffer_texture) {
      auto depth_texture = make_shared<Texture>(
          Texture::Meta{
              .width = meta.width,
              .height = meta.height,
              .internal_format = GL_DEPTH_COMPONENT,
              .input_format = GL_DEPTH_COMPONENT,
              .input_type = GL_FLOAT,
          },
          empty);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                             depth_texture->id, 0);
      depth_buffer = depth_texture;
    } else {
      unsigned int rbo = 0;
      glGenRenderbuffers(1, &rbo);
      glBindRenderbuffer(GL_RENDERBUFFER, rbo);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, meta.width,
                            meta.height);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                                GL_RENDERBUFFER, rbo);
      depth_buffer = rbo;
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      throw FramebufferException("framebuffer is not complete");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    SPDLOG_TRACE("Created framebuffer {} {}", meta.width, meta.height);
  }

  std::shared_ptr<Texture> recreate_color_attachment0() {
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
  void create_extra_color_attachment(std::shared_ptr<Texture> attachment) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);

    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT1 + color_attachments.size(),
                           GL_TEXTURE_2D, attachment->id, 0);
    color_attachments.emplace_back(attachment);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      throw FramebufferException("Framebuffer is not complete");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
  void set_draw_buffers(const std::vector<unsigned int> &draw_buffers) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);

    glDrawBuffers(draw_buffers.size(), draw_buffers.data());
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      throw FramebufferException("Framebuffer is not complete");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void start_capture() {
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    start_frame_width = viewport[2];
    start_frame_height = viewport[3];
    glViewport(0, 0, this->meta.width, this->meta.height);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  void end_capture() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, start_frame_width, start_frame_height);
  }

  std::shared_ptr<Texture> get_color_attachment0() { return color_attachment0; }
  std::vector<std::shared_ptr<Texture>> &get_color_attachments() {
    return color_attachments;
  }

  std::shared_ptr<Texture> get_depth_attachment() {
    if (meta.depthbuffer_texture) {
      return std::get<shared_ptr<Texture>>(this->depth_buffer);
    }
    throw FramebufferException(
        "Framebuffer does not use depth texture as depth attachment");
  }
  glm::ivec2 get_size() { return ivec2(meta.width, meta.height); }

public:
  ~Framebuffer() { glDeleteFramebuffers(1, &framebuffer_id); }

  Framebuffer(const Framebuffer &other) = delete;
  Framebuffer &operator=(Framebuffer &other) = delete;

  Framebuffer(Framebuffer &&other) noexcept :
      start_frame_width(other.start_frame_width),
      start_frame_height(other.start_frame_height),
      framebuffer_id{other.framebuffer_id},
      depth_buffer(std::move(other.depth_buffer)),
      meta(other.meta),
      color_attachment0{std::move(other.color_attachment0)},
      color_attachments(std::move(other.color_attachments)) {
    other.framebuffer_id = 0;
  }
  Framebuffer &operator=(Framebuffer &&other) noexcept {
    if (this != &other) {
      swap(this->color_attachment0, other.color_attachment0);
      swap(this->color_attachments, other.color_attachments);
      swap(this->depth_buffer, other.depth_buffer);
      swap(this->meta, other.meta);
      swap(this->framebuffer_id, other.framebuffer_id);
      swap(this->start_frame_width, other.start_frame_width);
      swap(this->start_frame_height, other.start_frame_height);
    }
    return *this;
  }
};
}; // namespace ale::graphics
