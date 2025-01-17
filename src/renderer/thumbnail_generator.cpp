//
// Created by Alether on 1/8/2025.
//

#include "thumbnail_generator.h"
#include "../camera.h"

namespace ale {
ThumbnailGenerator::ThumbnailGenerator(int thumbnail_width,
                                       int thumbnail_height)
    : framebuffer(Framebuffer::Meta{.width = thumbnail_width,
                                    .height = thumbnail_height}) {}

shared_ptr<Texture> ThumbnailGenerator::generate(StaticMesh static_mesh) {
  {
    framebuffer.start_frame();

    auto camera =
        Camera(ARCBALL, framebuffer.get_size().x, framebuffer.get_size().y,
               glm::vec3(3.0f, 5.0f, -7.0f));
    auto world = entt::registry{};
    // create model
    {
      const auto entity = world.create();
      world.emplace<Transform>(entity, Transform{});
      world.emplace<StaticMesh>(entity, static_mesh);
    }
    // create light
    {
      const auto entity = world.create();
      world.emplace<Transform>(
          entity, Transform{.translation = vec3(5.0f, 5.0f, 5.0f)});
      world.emplace<Light>(entity, Light{});
    }

    renderer.render(camera, world);

    framebuffer.end_frame();
  }
  return this->framebuffer.create_new_color_attachment0();
}
} // namespace ale