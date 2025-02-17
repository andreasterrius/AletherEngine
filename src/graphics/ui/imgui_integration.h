//
// Created by Alether on 1/7/2025.
//

#ifndef IMGUI_INTEGRATION_H
#define IMGUI_INTEGRATION_H

struct GLFWwindow;

namespace ale {
class ImguiIntegration {
  bool has_init = false;

public:
  void start_frame();

  void end_frame();

public:
  ImguiIntegration(GLFWwindow *raw, float content_scale);

  ImguiIntegration(const ImguiIntegration &other) = delete;

  ImguiIntegration(ImguiIntegration &&other) noexcept {
    has_init = other.has_init;
    other.has_init = false;
  }

  ImguiIntegration &operator=(ImguiIntegration other) {
    has_init = other.has_init;
    other.has_init = false;
    return *this;
  }

  ~ImguiIntegration();
};
} // namespace ale

#endif // IMGUI_INTEGRATION_H
