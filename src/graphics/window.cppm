//
// Created by Alether on 4/15/2024.
//

module;

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <functional>
#include <glm/glm.hpp>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

export module graphics:window;
import input;

using namespace glm;
using namespace std;

// callback declaration, defined below.
void mouse_button_callback(GLFWwindow *raw_window, int button, int action,
                           int mods);
void cursor_pos_callback(GLFWwindow *raw_window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow *raw_window, int width, int height);
void scroll_callback(GLFWwindow *raw_window, double x_offset, double y_offset);
void key_callback(GLFWwindow *raw_window, int key, int scancode, int action,
                  int mods);

export namespace ale::graphics {

void GLAPIENTRY debugCallback(GLenum source, GLenum type, GLuint id,
                              GLenum severity, GLsizei length,
                              const GLchar *message, const void *userParam) {
  std::cerr << "---------------" << std::endl;
  std::cerr << "Source: ";
  switch (source) {
    case GL_DEBUG_SOURCE_API:
      std::cerr << "API";
      break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
      std::cerr << "Window System";
      break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
      std::cerr << "Shader Compiler";
      break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
      std::cerr << "Third Party";
      break;
    case GL_DEBUG_SOURCE_APPLICATION:
      std::cerr << "Application";
      break;
    case GL_DEBUG_SOURCE_OTHER:
      std::cerr << "Other";
      break;
  }

  std::cerr << "\nType: ";
  switch (type) {
    case GL_DEBUG_TYPE_ERROR:
      std::cerr << "Error";
      break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
      std::cerr << "Deprecated Behavior";
      break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
      std::cerr << "Undefined Behavior";
      break;
    case GL_DEBUG_TYPE_PORTABILITY:
      std::cerr << "Portability";
      break;
    case GL_DEBUG_TYPE_PERFORMANCE:
      std::cerr << "Performance";
      break;
    case GL_DEBUG_TYPE_OTHER:
      std::cerr << "Other";
      break;
  }

  std::cerr << "\nID: " << id;
  std::cerr << "\nSeverity: ";
  switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
      std::cerr << "high";
      break;
    case GL_DEBUG_SEVERITY_MEDIUM:
      std::cerr << "medium";
      break;
    case GL_DEBUG_SEVERITY_LOW:
      std::cerr << "low";
      break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
      std::cerr << "notification";
      break;
  }

  std::cerr << "\nMessage: " << message << std::endl;
}

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

class Window : public input::WindowEventProducer {

  GLFWwindow *raw_window = nullptr;
  // std::unique_ptr<editor::ImguiIntegration> imgui;
  static bool first_window_init;

protected:
  Data data; // to be passed to callbacks as well.

public:
  Window(int width, int height, string caption) {
    // TODO: hardcoded for now, unless the need arise to separate this
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    this->raw_window =
        glfwCreateWindow(width, height, caption.c_str(), nullptr, nullptr);
    if (this->raw_window == nullptr) {
      throw WindowException("glfwCreateWindow returns null");
    }
    glfwMakeContextCurrent(this->raw_window);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
      throw WindowException("loading gl functions failed");
    }

    glfwSetWindowUserPointer(this->raw_window, this);
    this->data.width = width;
    this->data.height = height;
    this->data.last_resize_width = width;
    this->data.last_resize_height = height;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_FRAMEBUFFER_SRGB);

    glfwSetMouseButtonCallback(this->raw_window, mouse_button_callback);
    glfwSetCursorPosCallback(this->raw_window, cursor_pos_callback);
    glfwSetFramebufferSizeCallback(this->raw_window, framebuffer_size_callback);
    glfwSetScrollCallback(this->raw_window, scroll_callback);
    glfwSetKeyCallback(this->raw_window, key_callback);

    // this->imgui =
    //     make_unique<ImguiIntegration>(this->raw_window, get_content_scale());
  }

public:
  void set_default_inputs(DefaultInputs default_inputs) {
    this->data.default_inputs = default_inputs;
  }

  void set_debug(bool flag) {
    this->data.debug = flag;
    if (flag) {
      glEnable(GL_DEBUG_OUTPUT);
      glDebugMessageCallback(debugCallback, nullptr);
    } else {
      glDisable(GL_DEBUG_OUTPUT);
      glDebugMessageCallback(nullptr, nullptr);
    }
  }

  bool get_should_close() { return glfwWindowShouldClose(this->raw_window); }

public:
  void set_should_close(bool flag) {
    glfwSetWindowShouldClose(raw_window, flag);
  }

  void swap_buffer_and_poll_inputs() {
    glfwSwapBuffers(this->raw_window);
    glfwPollEvents();
  }

  ivec2 get_position() {
    int x = 0, y = 0;
    glfwGetWindowPos(raw_window, &x, &y);
    return ivec2(x, y);
  }

  ivec2 get_size() {
    int x = 0, y = 0;
    glfwGetWindowSize(raw_window, &x, &y);
    return ivec2(x, y);
  }

  vec2 get_cursor_pos_from_top_left() {
    auto v = get_cursor_pos();
    int wx, wy;
    glfwGetWindowPos(raw_window, &wx, &wy);
    return vec2(v.x + wx, v.y + wy);
  };

  vec2 get_cursor_pos() {
    double mouseX, mouseY;
    glfwGetCursorPos(raw_window, &mouseX, &mouseY);
    return vec2(mouseX, mouseY);
  }

  float get_content_scale() {
    float x = 0.0f;
    float y = 0.0f;
    glfwGetWindowContentScale((GLFWwindow *) raw_window, &x, &y);
    return x;
  }

  Data &get_data() { return data; }

  GLFWwindow *get() { return raw_window; }

public:
  // input callbacks
  void attach_mouse_button_callback(const function<void(int, int, int)> &func) {
    this->data.mouse_button_callback = func;
  }

  void attach_cursor_pos_callback(
      const function<void(double, double, double, double)> &func) {
    this->data.cursor_pos_callback = func;
  }

  void attach_framebuffer_size_callback(const function<void(int, int)> &func) {
    this->data.framebuffer_size_callback = func;
  }

  void attach_scroll_callback(const function<void(double, double)> &func) {
    this->data.scroll_callback = func;
  }

  void attach_key_callback(const function<void(int, int, int, int)> &func) {
    this->data.key_callback = func;
  }

public:
  void start_ui_frame() {
    // if (imgui != nullptr) {
    //   imgui->start_frame();
    // }
  }

  void end_ui_frame() {
    // glDisable(GL_FRAMEBUFFER_SRGB);
    // if (imgui != nullptr) {
    //   glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //   imgui->end_frame();
    // }
    // glEnable(GL_FRAMEBUFFER_SRGB);
  }

  // // TODO: Remove
  // GLFWwindow *get() { return raw_window; }

  ~Window() {
    glfwSetWindowUserPointer(raw_window, nullptr);
    glfwDestroyWindow(raw_window);
  }
};
} // namespace ale::graphics


// GLFW callback funcs
using namespace ale::graphics;
void mouse_button_callback(GLFWwindow *raw_window, int button, int action,
                           int mods) {
  Window *window = static_cast<Window *>(glfwGetWindowUserPointer(raw_window));
  Data &data = window->get_data();

  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    if (data.is_cursor_enabled) {
      // enable -> disable
      glfwSetInputMode(raw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      data.is_cursor_enabled = false;
    } else {
      // disable -> enable
      glfwSetInputMode(raw_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      data.is_cursor_enabled = true;
    }
  }

  if (data.mouse_button_callback != nullptr) {
    data.mouse_button_callback(button, action, mods);
  }

  for (auto &listener: window->get_listeners()) {
    listener->mouse_button_callback(button, action, mods);
  }
}

void cursor_pos_callback(GLFWwindow *raw_window, double xpos, double ypos) {
  Window *window = static_cast<Window *>(glfwGetWindowUserPointer(raw_window));
  Data &data = window->get_data();

  data.cursor_offset_x = xpos - data.cursor_last_x;
  data.cursor_offset_y =
      data.cursor_last_y -
      ypos; // reversed since y-coordinates go from bottom to top
  data.cursor_last_x = xpos;
  data.cursor_last_y = ypos;

  if (data.cursor_pos_callback != nullptr) {
    data.cursor_pos_callback(xpos, ypos, data.cursor_offset_x,
                             data.cursor_offset_y);
  }

  for (auto &listener: window->get_listeners()) {
    listener->cursor_pos_callback(xpos, ypos, data.cursor_offset_x,
                                  data.cursor_offset_y);
  }
}

void framebuffer_size_callback(GLFWwindow *raw_window, int width, int height) {
  Window *window = static_cast<Window *>(glfwGetWindowUserPointer(raw_window));
  Data &data = window->get_data();
  data.width = width;
  data.height = height;

  // make sure the viewport matches the new window dimensions; note that width
  // and height will be significantly larger than specified on retina
  // displays.
  glViewport(0, 0, width, height);

  if (data.framebuffer_size_callback) {
    data.framebuffer_size_callback(width, height);
  }

  for (auto &listener: window->get_listeners()) {
    listener->framebuffer_size_callback(width, height);
  }
}

void scroll_callback(GLFWwindow *raw_window, double x_offset, double y_offset) {
  Window *window = static_cast<Window *>(glfwGetWindowUserPointer(raw_window));
  Data &data = window->get_data();

  if (data.scroll_callback != nullptr) {
    data.scroll_callback(x_offset, y_offset);
  }

  for (auto &listener: window->get_listeners()) {
    listener->scroll_callback(x_offset, y_offset);
  }
}
void key_callback(GLFWwindow *raw_window, int key, int scancode, int action,
                  int mods) {
  Window *window = static_cast<Window *>(glfwGetWindowUserPointer(raw_window));
  Data &data = window->get_data();

  if (data.key_callback != nullptr) {
    data.key_callback(key, scancode, action, mods);
  }

  for (auto &listener: window->get_listeners()) {
    listener->key_callback(key, scancode, action, mods);
  }
}
