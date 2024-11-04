//
// Created by Alether on 4/15/2024.
//

#ifndef ALETHERENGINE_WINDOW_H
#define ALETHERENGINE_WINDOW_H

#include <functional>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <stdexcept>

using namespace std;
class GLFWwindow;


namespace ale {
class WindowException final : public runtime_error {
public:
    explicit WindowException(const string &msg): runtime_error(msg) {
    }
};

struct DefaultInputs {
    int keyboard_key_to_disable_cursor = -1;
    int keyboard_key_to_enable_cursor = -1;
};

struct Data {
    bool debug = false;
    DefaultInputs default_inputs;

    int width, height;

    // Cursor Information
    bool is_cursor_enabled = true;
    double cursor_last_x = 0.0, cursor_last_y = 0.0; //exact coord on window
    double cursor_offset_x = 0.0, cursor_offset_y = 0.0; //diff between last input and curr input

    function<void(int, int, int)> mouse_button_callback = nullptr;
    function<void(double, double, double, double)> cursor_pos_callback = nullptr;
    function<void(int, int)> framebuffer_size_callback = nullptr;
    function<void(double, double)> scroll_callback = nullptr;
};

class Window {
    GLFWwindow *raw_window = nullptr;
    Data data; //to be passed to callbacks as well.

public:
    Window(int width, int height, string caption);

    void set_default_inputs(DefaultInputs default_inputs);

    void set_debug(bool flag);

    bool should_close();

    void swap_buffer_and_poll_inputs();

    int get_width();

    int get_height();

    // input callbacks
    void attach_mouse_button_callback(const function<void(int, int, int)> &func);

    void attach_cursor_pos_callback(const function<void(double, double, double, double)> &func);

    void attach_framebuffer_size_callback(const function<void(int, int)> &func);

    void attach_scroll_callback(const function<void(double, double)> &func);

    //TODO: Remove
    GLFWwindow *get() {
        return raw_window;
    }

    ~Window();
};

// GLFW callback funcs
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos);

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void scroll_callback(GLFWwindow *window, double x_offset, double y_offset);

void GLAPIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
} // namespace ale

#endif // ALETHERENGINE_WINDOW_H
