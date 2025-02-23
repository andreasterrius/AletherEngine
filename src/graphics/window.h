//
// Created by Alether on 4/15/2024.
//

#ifndef ALETHERENGINE_WINDOW_H
#define ALETHERENGINE_WINDOW_H

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
#include "src/input_handling/window_event.h"
#include "ui/imgui_integration.h"
#include <functional>
#include <glm/glm.hpp>
#include <memory>
#include <stdexcept>
#include <string>

class GLFWwindow;

namespace ale {
class WindowException final : public std::runtime_error {
public:
  explicit WindowException(const std::string &msg) : std::runtime_error(msg) {}
};

struct DefaultInputs {
  int keyboard_key_to_disable_cursor = -1;
  int keyboard_key_to_enable_cursor = -1;
};

struct Data {
  bool debug = false;
  DefaultInputs default_inputs;

  int width, height;

  // Resize information
  int last_resize_width, last_resize_height;

  // Cursor Information
  bool is_cursor_enabled = true;
  double cursor_last_x = 0.0, cursor_last_y = 0.0; // exact coord on window
  double cursor_offset_x = 0.0,
         cursor_offset_y = 0.0; // diff last x curr input

  std::function<void(int, int, int)> mouse_button_callback = nullptr;
  std::function<void(double, double, double, double)> cursor_pos_callback =
      nullptr;
  std::function<void(int, int)> framebuffer_size_callback = nullptr;
  std::function<void(double, double)> scroll_callback = nullptr;
  std::function<void(int, int, int, int)> key_callback = nullptr;
};

class Window : public WindowEventProducer {
  GLFWwindow *raw_window = nullptr;
  std::unique_ptr<ImguiIntegration> imgui;
  static bool first_window_init;

protected:
  Data data; // to be passed to callbacks as well.

public:
  Window(int width, int height, std::string caption);

public:
  void set_default_inputs(DefaultInputs default_inputs);
  void set_debug(bool flag);
  void set_should_close(bool flag);

public:
  bool get_should_close();
  glm::ivec2 get_position();
  glm::ivec2 get_size();
  glm::vec2 get_cursor_pos_from_top_left();
  glm::vec2 get_cursor_pos();
  float get_content_scale(); // take x since x/y most likely be equal
  Data &get_data();

public:
  void swap_buffer_and_poll_inputs();
  // input callbacks
  void
  attach_mouse_button_callback(const std::function<void(int, int, int)> &func);

  void attach_cursor_pos_callback(
      const std::function<void(double, double, double, double)> &func);

  void
  attach_framebuffer_size_callback(const std::function<void(int, int)> &func);

  void attach_scroll_callback(const std::function<void(double, double)> &func);

  void attach_key_callback(const std::function<void(int, int, int, int)> &func);

public:
  void start_ui_frame();

  void end_ui_frame();

  // // TODO: Remove
  // GLFWwindow *get() { return raw_window; }

  ~Window();
};

// GLFW callback funcs
void mouse_button_callback(GLFWwindow *raw_window, int button, int action,
                           int mods);

void cursor_pos_callback(GLFWwindow *raw_window, double xpos, double ypos);

void framebuffer_size_callback(GLFWwindow *raw_window, int width, int height);

void scroll_callback(GLFWwindow *raw_window, double x_offset, double y_offset);

void key_callback(GLFWwindow *raw_window, int key, int scancode, int action,
                  int mods);

void GLAPIENTRY debugCallback(GLenum source, GLenum type, GLuint id,
                              GLenum severity, GLsizei length,
                              const GLchar *message, const void *userParam);
} // namespace ale

#endif // ALETHERENGINE_WINDOW_H
