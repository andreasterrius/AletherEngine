//
// Created by Alether on 1/8/2025.
//
module;

#include <entt/entt.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>

export module thumbnail_generator;
import basic_renderer;
import framebuffer;
import light;
import camera;
import static_mesh;
import texture;
import transform;
import material;

using namespace ale::graphics::renderer;
using namespace ale::data;
using namespace glm;
using namespace std;

constexpr int DEFAULT_THUMBNAIL_WIDTH = 400;
constexpr int DEFAULT_THUMBNAIL_HEIGHT = 400;

export namespace ale::graphics {
class ThumbnailGenerator {
  BasicRenderer renderer;
  Framebuffer framebuffer;

public:
  explicit ThumbnailGenerator(int thumbnail_width = DEFAULT_THUMBNAIL_WIDTH,
                              int thumbnail_height = DEFAULT_THUMBNAIL_HEIGHT) :
      framebuffer(Framebuffer::Meta{.width = thumbnail_width,
                                    .height = thumbnail_height,
                                    .color_space = Framebuffer::SRGB}) {};

  shared_ptr<Texture> generate(StaticMesh static_mesh) {
    {
      framebuffer.start_capture();

      auto camera =
          Camera(ARCBALL, framebuffer.get_size().x, framebuffer.get_size().y,
                 glm::vec3(3.0f, 5.0f, 7.0f));
      auto world = entt::registry{};
      // create model
      {
        const auto entity = world.create();
        world.emplace<Transform>(entity, Transform{});
        world.emplace<StaticMesh>(entity, static_mesh);
        world.emplace<BasicMaterial>(entity, BasicMaterial{});
      }
      // create light
      {
        const auto entity = world.create();
        world.emplace<Transform>(
            entity, Transform{.translation = vec3(5.0f, 5.0f, 5.0f)});
        world.emplace<Light>(entity,
                             Light{.attenuation = vec3(1.0, 0.022, 0.0019)});
        world.emplace<BasicMaterial>(entity, BasicMaterial{});
      }

      renderer.render(camera, world);

      // wait for the render to be done before we continue
      // perhaps there are other synchronization object that better fulfill
      // this?
      glFinish();

      framebuffer.end_capture();
    }
    return this->framebuffer.recreate_color_attachment0();
  }
};
} // namespace ale::graphics
