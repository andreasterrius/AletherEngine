#include "basic_renderer.h"

#include "src/data/file_system.h"
#include "src/graphics/static_mesh.h"

#include <format>

namespace ale {

using namespace glm;
using afs = ale::FileSystem;

BasicRenderer::BasicRenderer(ivec2 screen_size)
    : color_shader(
          afs::root("resources/shaders/renderer/basic_renderer.vs").c_str(),
          afs::root("resources/shaders/renderer/basic_renderer.fs").c_str()),
      single_black_pixel_texture(
          afs::root("resources/textures/default/black1x1.png")),
      deferred_framebuffer(
          Framebuffer::Meta{.width = screen_size.x, .height = screen_size.y}) {

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
  deferred_framebuffer.set_draw_buffers(
      {GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3});
  glEnable(GL_CULL_FACE);
}

BasicRenderer::~BasicRenderer() {
  if (event_producer)
    event_producer->remove_listener(this);
}

void BasicRenderer::add_listener(WindowEventProducer *event_producer) {
  this->event_producer = event_producer;
  this->event_producer->add_listener(this);
}

void BasicRenderer::render(Camera &camera, entt::registry &world) {
  deferred_framebuffer.start_capture();
  glClearColor(135.0 / 255, 206.0 / 255, 235.0 / 255, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  color_shader.use();
  color_shader.setMat4("projection", camera.get_projection_matrix());
  color_shader.setMat4("view", camera.get_view_matrix());
  color_shader.setVec3("viewPos", camera.Position);

  auto light_view = world.view<Transform, Light>();
  int light_index = 0;
  for (const auto &[entity, transform, light] : light_view.each()) {
    color_shader.setVec3(format("lights[{}].position", light_index),
                         transform.translation);
    color_shader.setVec3(format("lights[{}].color", light_index), light.color);
    color_shader.setFloat(format("lights[{}].radius", light_index),
                          light.radius);
    color_shader.setVec3(format("lights[{}].attenuation", light_index),
                         light.attenuation);
    light_index += 1;
  }
  color_shader.setInt("numLights", light_index);

  // Handle shadows, can only handle 1 sdf model packed for now.
  auto sdf_model_packed = std::shared_ptr<SdfModelPacked>(nullptr);
  auto entries = vector<pair<Transform, vector<unsigned int>>>();
  auto shadow_view = world.view<Transform, StaticMesh>();
  for (auto [entity, transform, static_mesh] : shadow_view.each()) {
    auto [packed, packed_index] = static_mesh.get_model_shadow();
    if (static_mesh.get_cast_shadow() && packed != nullptr) {
      if (sdf_model_packed != nullptr &&
          sdf_model_packed.get() != packed.get()) {
        throw BasicRendererException("multiple different sdf model packed on "
                                     "basic renderer not supported");
      }
      sdf_model_packed = packed;
      entries.emplace_back(transform, packed_index);
    }
  }
  if (sdf_model_packed != nullptr) {
    sdf_model_packed->bind_to_shader(color_shader, entries);
  }
  // End handle shadows

  // Render static mesh
  const auto view = world.view<Transform, StaticMesh, BasicMaterial>();
  for (auto [entity, transform, static_mesh, material] : view.each()) {
    color_shader.setMat4("model", transform.get_model_matrix());

    color_shader.setVec4("diffuseColor", vec4(1.0, 1.0, 1.0, 0.0));
    set_texture_with_default("diffuseTexture", 0,
                             material.diffuse_texture.get());

    if (sdf_model_packed != nullptr) {
      auto &atlas = sdf_model_packed->get_texture_atlas();
      for (int i = 0; i < 6; ++i) {
        if (i < atlas.size()) {
          set_texture_with_default(format("atlas[{}]", i), i + 6, &atlas.at(i));
        } else {
          set_texture_with_default(format("atlas[{}]", i), i + 6, nullptr);
        }
      }
    }

    static_mesh.get_model()->draw(color_shader);
  }
  deferred_framebuffer.end_capture();
}
void BasicRenderer::set_texture_with_default(string name, int location,
                                             const Texture *texture) const {

  color_shader.setTexture2D(name, location,
                            texture == nullptr ? single_black_pixel_texture.id
                                               : texture->id);
}

void BasicRenderer::mouse_button_callback(int button, int action, int mods) {}
void BasicRenderer::cursor_pos_callback(double xpos, double ypos,
                                        double xoffset, double yoffset) {}
void BasicRenderer::framebuffer_size_callback(int width, int height) {
  this->deferred_framebuffer =
      Framebuffer(Framebuffer::Meta{.width = width, .height = height});
}
void BasicRenderer::scroll_callback(double x_offset, double y_offset) {}
void BasicRenderer::key_callback(int key, int scancode, int action, int mods) {}
} // namespace ale