//
// Created by Alether on 4/15/2024.
//

#include "window.h"

void ale::DestroyGLFWwindow::operator()(GLFWwindow *ptr) { glfwDestroyWindow(ptr); }

unique_ptr<GLFWwindow, ale::DestroyGLFWwindow> ale::createWindow(int width, int height) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(width, height, "AletherEngine", NULL, NULL);
    if (window == nullptr) {
        return nullptr;
    }
    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        return nullptr;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    return unique_ptr<GLFWwindow, DestroyGLFWwindow>(window);
}

