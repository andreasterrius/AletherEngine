module;
#include <entt/entt.hpp>
#include <format>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include "src/data/serde/glm.h"
#include "src/graphics/camera.h"
#include "src/graphics/framebuffer.h"
#include "src/graphics/light.h"
#include "src/graphics/sdf/sdf_model_packed.h"
#include "src/graphics/shader.h"
#include "src/graphics/static_mesh.h"

export module deferred_renderer;

import material;
import file_system;

export namespace ale {
using namespace input_handling;

class DeferredRendererException final : public std::runtime_error {
public:
  explicit DeferredRendererException(const std::string &msg) :
      runtime_error(msg) {}
};

class DeferredRenderer : public WindowEventListener {
  using afs = ale::FileSystem;

  struct FirstPassData {
    shared_ptr<SdfModelPacked> sdf_model_packed = nullptr;
    vector<pair<Transform, vector<unsigned int>>> entries;
  };

private:
  Shader first_pass;
  Shader second_pass;
  Texture single_black_pixel_texture;

  Framebuffer deferred_framebuffer;
  TextureRenderer texture_renderer;
  WindowEventProducer *event_producer = nullptr;

  FirstPassData first_pass_data;

public:
  DeferredRenderer(glm::ivec2 screen_size) :
      first_pass(
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

  ~DeferredRenderer() {
    if (event_producer)
      event_producer->remove_listener(this);
  }

  void add_listener(WindowEventProducer *event_producer) {
    this->event_producer = event_producer;
    this->event_producer->add_listener(this);
  }

  void render_both_pass(Camera &camera, entt::registry &world) {
    render_first_pass(camera, world);
    render_second_pass(camera, world);
  }

  void render_first_pass(Camera &camera, entt::registry &world) {
    using namespace std;
    // can only accommodate 1 sdf_model_packed for now
    first_pass_data.sdf_model_packed = nullptr;
    first_pass_data.entries = vector<pair<Transform, vector<unsigned int>>>();
    deferred_framebuffer.start_capture();
    first_pass.use();

    first_pass.setMat4("projection", camera.get_projection_matrix());
    first_pass.setMat4("view", camera.get_view_matrix());
    const auto view = world.view<Transform, StaticMesh, BasicMaterial>();
    for (auto [entity, transform, static_mesh, material]: view.each()) {

      pass_basic_material(first_pass, entity, transform, material);
      pass_shadow(first_pass, transform, static_mesh);
    }

    const auto pbrs = world.view<Transform, StaticMesh, PBRMaterial>();
    for (auto [entity, transform, static_mesh, material]: pbrs.each()) {
      pass_pbr_material(first_pass, entity, material);
      pass_shadow(first_pass, transform, static_mesh);
    }

    deferred_framebuffer.end_capture();
  }

  void render_second_pass(Camera &camera, entt::registry &world) {
    // clear color with ambient light
    auto ambient_view = world.view<AmbientLight>();
    auto ambient_color = glm::vec3(0.0f);
    auto ambient_intensity = 0.0f;
    auto background_color = glm::vec3(0.0f);
    second_pass.use();
    for (const auto &[entity, ambient_light]: ambient_view.each()) {
      ambient_color = ambient_light.color;
      ambient_intensity = ambient_light.intensity;
      background_color = ambient_light.background_color;
      break;
    }
    glClearColor(background_color.r, background_color.g, background_color.b,
                 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    second_pass.setVec3("viewPos", camera.Position);
    second_pass.setVec3("ambientColor", ambient_color);
    second_pass.setFloat("ambientIntensity", ambient_intensity);

    auto light_view = world.view<Transform, Light>();
    int light_index = 0;
    for (const auto &[entity, transform, light]: light_view.each()) {
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

    if (first_pass_data.sdf_model_packed != nullptr) {
      first_pass_data.sdf_model_packed->bind_to_shader(
          second_pass, first_pass_data.entries, 5);
    }

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    texture_renderer.render_quad(second_pass);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
  }

private:
  void pass_basic_material(Shader &first_pass, entt::entity &entity,
                           Transform &transform, BasicMaterial &material) {
    first_pass.setMat4("model", transform.get_model_matrix());
    first_pass.setInt("entityId", to_integral(entity));

    pass_vec3("diffuse", material.diffuse_color, material.diffuse_texture);
    pass_float("specular", material.specular_color, material.specular_texture);

    // first_pass.setVec3("diffuseColor", material.diffuse_color);
    // first_pass.setTexture2D("diffuseTexture", 0,
    //                         material.diffuse_texture == nullptr
    //                             ? single_black_pixel_texture.id
    //                             : material.diffuse_texture->id);
    // first_pass.setFloat("specularColor", material.specular_color);
    // first_pass.setTexture2D("specularTexture", 1,
    //                         material.specular_texture == nullptr
    //                             ? single_black_pixel_texture.id
    //                             : material.specular_texture->id);
  }

  void pass_pbr_material(Shader &first_pass, entt::entity &entity,
                         PBRMaterial &material) {}

  void pass_shadow(Shader &first_pass, Transform &transform,
                   StaticMesh &static_mesh) {
    static_mesh.get_model()->draw(first_pass);
    auto [packed, packed_index] = static_mesh.get_model_shadow();
    if (static_mesh.get_cast_shadow() && packed != nullptr) {
      if (first_pass_data.sdf_model_packed != nullptr &&
          first_pass_data.sdf_model_packed.get() != packed.get()) {
        throw DeferredRendererException(
            "multiple different sdf model packed on "
            "deferred renderer not supported");
      }
      first_pass_data.sdf_model_packed = packed;
      first_pass_data.entries.emplace_back(transform, packed_index);
    }
  }

  void pass_float(string name, float color, shared_ptr<Texture> texture) {
    first_pass.setFloat(name + "Color", color);
    first_pass.setTexture2D(name + "Texture", 1,
                            texture == nullptr ? single_black_pixel_texture.id
                                               : texture->id);
  }
  void pass_vec3(string name, glm::vec3 color, shared_ptr<Texture> texture) {
    first_pass.setVec3(name + "Color", color);
    first_pass.setTexture2D(name + "Texture", 1,
                            texture == nullptr ? single_black_pixel_texture.id
                                               : texture->id);
  }

public:
  void mouse_button_callback(int button, int action, int mods) override {}
  void cursor_pos_callback(double xpos, double ypos, double xoffset,
                           double yoffset) override {}
  void framebuffer_size_callback(int width, int height) override {}
  void scroll_callback(double x_offset, double y_offset) override {}
  void key_callback(int key, int scancode, int action, int mods) override {}
};
}; // namespace ale
