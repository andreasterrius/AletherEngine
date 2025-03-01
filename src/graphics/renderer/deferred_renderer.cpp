#include "deferred_renderer.h"

#include "src/data/file_system.h"
#include "src/graphics/static_mesh.h"

namespace ale {

using namespace glm;
using afs = ale::FileSystem;

DeferredRenderer::DeferredRenderer(ivec2 screen_size)
    : first_pass(
          afs::root(
              "resources/shaders/renderer/deferred_renderer/first_pass.vs")
              .c_str(),
          afs::root(
              "resources/shaders/renderer/deferred_renderer/first_pass.fs")
              .c_str()),
      second_pass(
          afs::root(
              "resources/shaders/renderer/deferred_renderer/second_pass.vs")
              .c_str(),
          afs::root(
              "resources/shaders/renderer/deferred_renderer/second_pass.fs")
              .c_str()),
      single_black_pixel_texture(
          afs::root("resources/textures/default/black1x1.png")),
      deferred_framebuffer(Framebuffer::Meta{.width = screen_size.x,
                                             .height = screen_size.y,
                                             .depthbuffer_texture = true}) {

  // position buffer
  deferred_framebuffer.create_extra_color_attachment(make_shared<Texture>(
      Texture::Meta{
          .width = screen_size.x,
          .height = screen_size.y,
          .internal_format = GL_RGBA16F,
          .input_format = GL_RGBA,
          .input_type = GL_FLOAT,
          .min_filter = GL_NEAREST,
          .max_filter = GL_NEAREST,
      },
      nullptr));
  // normal buffer
  deferred_framebuffer.create_extra_color_attachment(make_shared<Texture>(
      Texture::Meta{
          .width = screen_size.x,
          .height = screen_size.y,
          .internal_format = GL_RGBA16F,
          .input_format = GL_RGBA,
          .input_type = GL_FLOAT,
          .min_filter = GL_NEAREST,
          .max_filter = GL_NEAREST,
      },
      nullptr));
  // color + specular
  deferred_framebuffer.create_extra_color_attachment(make_shared<Texture>(
      Texture::Meta{
          .width = screen_size.x,
          .height = screen_size.y,
          .internal_format = GL_RGBA,
          .input_format = GL_RGBA,
          .input_type = GL_UNSIGNED_BYTE,
          .min_filter = GL_NEAREST,
          .max_filter = GL_NEAREST,
      },
      nullptr));
  deferred_framebuffer.create_extra_color_attachment(make_shared<Texture>(
      Texture::Meta{
          .width = screen_size.x,
          .height = screen_size.y,
          .internal_format = GL_R32UI,
          .input_format = GL_RED_INTEGER,
          .input_type = GL_INT,
          .min_filter = GL_NEAREST,
          .max_filter = GL_NEAREST,
      },
      nullptr));
  deferred_framebuffer.set_draw_buffers(
      {GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
       GL_COLOR_ATTACHMENT4});
  glEnable(GL_CULL_FACE);
}

DeferredRenderer::~DeferredRenderer() {
  if (event_producer)
    event_producer->remove_listener(this);
}

void DeferredRenderer::add_listener(WindowEventProducer *event_producer) {
  this->event_producer = event_producer;
  this->event_producer->add_listener(this);
}

void DeferredRenderer::render(Camera &camera, entt::registry &world) {

  // can only accommodate 1 sdf_model_packed for now
  shared_ptr<SdfModelPacked> sdf_model_packed = nullptr;
  auto entries = vector<pair<Transform, vector<unsigned int>>>();

  deferred_framebuffer.start_capture();
  {
    first_pass.use();

    first_pass.setMat4("projection", camera.get_projection_matrix());
    first_pass.setMat4("view", camera.get_view_matrix());
    const auto view = world.view<Transform, StaticMesh, BasicMaterial>();
    for (auto [entity, transform, static_mesh, material] : view.each()) {
      first_pass.setMat4("model", transform.get_model_matrix());
      first_pass.setInt("entityId", to_integral(entity));

      first_pass.setVec3("diffuseColor", material.diffuse_color);
      first_pass.setTexture2D("diffuseTexture", 0,
                              material.diffuse_texture == nullptr
                                  ? single_black_pixel_texture.id
                                  : material.diffuse_texture->id);
      first_pass.setFloat("specularColor", 0.0f);
      first_pass.setTexture2D("specularTexture", 1,
                              material.specular_texture == nullptr
                                  ? single_black_pixel_texture.id
                                  : material.specular_texture->id);

      static_mesh.get_model()->draw(first_pass);

      auto [packed, packed_index] = static_mesh.get_model_shadow();
      if (static_mesh.get_cast_shadow() && packed != nullptr) {
        if (sdf_model_packed != nullptr &&
            sdf_model_packed.get() != packed.get()) {
          throw DeferredRendererException(
              "multiple different sdf model packed on "
              "deferred renderer not supported");
        }
        sdf_model_packed = packed;
        entries.emplace_back(transform, packed_index);
      }
    }
  }
  deferred_framebuffer.end_capture();

  {
    glClearColor(135.0 / 255, 206.0 / 255, 235.0 / 255, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    second_pass.use();
    second_pass.setVec3("viewPos", camera.Position);

    auto light_view = world.view<Transform, Light>();
    int light_index = 0;
    for (const auto &[entity, transform, light] : light_view.each()) {
      second_pass.setVec3(format("lights[{}].position", light_index),
                          transform.translation);
      second_pass.setVec3(format("lights[{}].color", light_index), light.color);
      second_pass.setFloat(format("lights[{}].radius", light_index),
                           light.radius);
      second_pass.setVec3(format("lights[{}].attenuation", light_index),
                          light.attenuation);
      light_index += 1;
    }
    second_pass.setInt("numLights", light_index);

    const auto &attachments = deferred_framebuffer.get_color_attachments();
    second_pass.setTexture2D("gPosition", 0, attachments.at(0)->id);
    second_pass.setTexture2D("gNormal", 1, attachments.at(1)->id);
    second_pass.setTexture2D("gAlbedoSpec", 2, attachments.at(2)->id);
    second_pass.setTexture2D("gEntityId", 3, attachments.at(3)->id);
    second_pass.setTexture2D("gDepth", 4,
                             deferred_framebuffer.get_depth_attachment()->id);

    if (sdf_model_packed != nullptr) {
      sdf_model_packed->bind_to_shader(second_pass, entries, 5);
    }

    glEnable(GL_BLEND);
    texture_renderer.render_quad(second_pass);
    glDisable(GL_BLEND);
  }
}

void DeferredRenderer::mouse_button_callback(int button, int action, int mods) {
}
void DeferredRenderer::cursor_pos_callback(double xpos, double ypos,
                                           double xoffset, double yoffset) {}
void DeferredRenderer::framebuffer_size_callback(int width, int height) {
  // this->deferred_framebuffer =
  //     Framebuffer(Framebuffer::Meta{.width = width, .height = height});
}
void DeferredRenderer::scroll_callback(double x_offset, double y_offset) {}
void DeferredRenderer::key_callback(int key, int scancode, int action,
                                    int mods) {}
} // namespace ale