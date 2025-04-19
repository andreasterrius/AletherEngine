//
// Created by Alether on 2/20/2025.
//
#include "camera.h"

#include <GLFW/glfw3.h>
#include <iostream>

namespace ale {
Camera::Camera(Camera_InputType inputType, int width, int height,
               glm::vec3 position, glm::vec3 up, float yaw, float pitch) :
    Front(glm::vec3(0.0f, 0.0f, -1.0f)),
    MovementSpeed(SPEED),
    MouseSensitivity(SENSITIVITY),
    Zoom(ZOOM),
    inputType(inputType),
    Width(width),
    Height(height) {
  Position = position;
  WorldUp = up;
  Yaw = yaw;
  Pitch = pitch;
  updateCameraVectors();
}

Camera::Camera(float posX, float posY, float posZ, float upX, float upY,
               float upZ, float yaw, float pitch) :
    Front(glm::vec3(0.0f, 0.0f, -1.0f)),
    MovementSpeed(SPEED),
    MouseSensitivity(SENSITIVITY),
    Zoom(ZOOM),
    inputType(FPS) {
  Position = glm::vec3(posX, posY, posZ);
  WorldUp = glm::vec3(upX, upY, upZ);
  Yaw = yaw;
  Pitch = pitch;
  updateCameraVectors();
}

Camera::~Camera() {
  if (event_producer != nullptr)
    event_producer->remove_listener(this);
}

void Camera::add_listener(WindowEventProducer *event_producer) {
  this->event_producer = event_producer;
  this->event_producer->add_listener(this);
}

void Camera::set_handle_input(bool handle_input) {
  this->handle_input = handle_input;
}

glm::mat4 Camera::get_view_matrix() const {
  return glm::lookAt(Position, Position + Front, Up);
}
void Camera::set_look_at(glm::vec3 point) {
  this->arcballInput.ArcballTarget = point;
  updateCameraVectors();
}

glm::mat4 Camera::get_projection_matrix(float screenWidth,
                                        float screenHeight) const {
  return glm::perspective(glm::radians(Zoom), screenWidth / screenHeight, 0.1f,
                          100.0f);
}

glm::mat4 Camera::get_projection_matrix() const {
  return get_projection_matrix(this->Width, this->Height);
}

void Camera::mouse_button_callback(int button, int action, int mods) {}
void Camera::scroll_callback(double x_offset, double y_offset) {
  ProcessMouseScroll(y_offset);
}
void Camera::cursor_pos_callback(double xpos, double ypos, double xoffset,
                                 double yoffset) {
  ProcessMouseMovement(xoffset, yoffset);
}
void Camera::framebuffer_size_callback(int width, int height) {}
void Camera::key_callback(int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_LEFT_ALT && action == GLFW_PRESS)
    ProcessKeyboardArcball(true);
  else if (key == GLFW_KEY_LEFT_ALT && action == GLFW_RELEASE)
    ProcessKeyboardArcball(false);
}

} // namespace ale
