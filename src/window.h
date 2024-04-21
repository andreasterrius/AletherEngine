//
// Created by Alether on 4/15/2024.
//

#ifndef ALETHERENGINE_WINDOW_H
#define ALETHERENGINE_WINDOW_H

#include <glad/glad.h>
#include "GLFW/glfw3.h"
#include <memory>

using namespace std;
class GLFWwindow;

namespace ale {

struct DestroyGLFWwindow {
    void operator()(GLFWwindow *ptr) { glfwDestroyWindow(ptr); }
};

unique_ptr<GLFWwindow, DestroyGLFWwindow> createWindow(int width, int height);

} // namespace ale

#endif // ALETHERENGINE_WINDOW_H
