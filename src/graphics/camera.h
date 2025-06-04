#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

import window_event;

namespace ale {

struct Camera_ArcballInput {
  // Used only if input type is arcball
  glm::vec3 ArcballTarget = glm::vec3();

  glm::vec2 CurrentMousePos = glm::vec2();

  // mutable input states
  bool ShouldOrbit = false;
};

enum Camera_InputType { FPS, ARCBALL };

// Defines several possible options for camera movement. Used as abstraction to
// stay away from window-system specific input methods
enum Camera_Movement { FORWARD, BACKWARD, LEFT, RIGHT };

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

// An abstract camera class that processes input and calculates the
// corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera : public input_handling::WindowEventListener {
public:
  // camera Attributes
  glm::vec3 Position;
  glm::vec3 Front;
  glm::vec3 Up;
  glm::vec3 Right;
  glm::vec3 WorldUp;
  // euler Angles
  float Yaw;
  float Pitch;
  // camera options
  float MovementSpeed;
  float MouseSensitivity;
  float Zoom;

  int Width;
  int Height;

  Camera_InputType inputType;
  Camera_ArcballInput arcballInput;
  bool handle_input = true;

  // non owning
  input_handling::WindowEventProducer *event_producer = nullptr;

  // constructor with vectors
  Camera(Camera_InputType inputType, int width, int height,
         glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
         glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW,
         float pitch = PITCH);

  // constructor with scalar values
  Camera(float posX, float posY, float posZ, float upX, float upY, float upZ,
         float yaw, float pitch);

  ~Camera() override;

  void add_listener(input_handling::WindowEventProducer *event_producer);

  void set_handle_input(bool handle_input);

  // returns the view matrix calculated using Euler Angles and the LookAt Matrix
  glm::mat4 get_view_matrix() const;

  void set_look_at(glm::vec3 point);

  glm::mat4 get_projection_matrix(float screenWidth, float screenHeight) const;

  glm::mat4 get_projection_matrix() const;

  // processes input received from any keyboard-like input system. Accepts input
  // parameter in the form of camera defined ENUM (to abstract it from windowing
  // systems)
  void ProcessKeyboardFPS(Camera_Movement direction, float deltaTime) {
    if (this->inputType == FPS) {
      float velocity = MovementSpeed * deltaTime;
      if (direction == FORWARD)
        Position += Front * velocity;
      if (direction == BACKWARD)
        Position -= Front * velocity;
      if (direction == LEFT)
        Position -= Right * velocity;
      if (direction == RIGHT)
        Position += Right * velocity;
    }
  }

  void ProcessKeyboardArcball(bool ShouldOrbit) {
    if (this->inputType == ARCBALL) {
      this->arcballInput.ShouldOrbit = ShouldOrbit;
    }
  }

  // processes input received from a mouse input system. Expects the offset
  // value in both the x and y direction.
  void ProcessMouseMovement(float xoffset, float yoffset,
                            GLboolean constrainPitch = true) {
    if (this->inputType == FPS) {
      xoffset *= MouseSensitivity;
      yoffset *= MouseSensitivity;

      Yaw += xoffset;
      Pitch += yoffset;

      // make sure that when pitch is out of bounds, screen doesn't get flipped
      if (constrainPitch) {
        if (Pitch > 89.0f)
          Pitch = 89.0f;
        if (Pitch < -89.0f)
          Pitch = -89.0f;
      }
    } else if (this->inputType == ARCBALL && this->arcballInput.ShouldOrbit) {
      glm::qua goUp = glm::angleAxis(glm::radians(yoffset), this->Right);
      glm::qua goRight = glm::angleAxis(glm::radians(-xoffset), this->Up);
      glm::qua combined = goUp * goRight;

      glm::vec3 pivotToPosition =
          this->Position - this->arcballInput.ArcballTarget;
      pivotToPosition = combined * pivotToPosition;

      this->Position = this->arcballInput.ArcballTarget + pivotToPosition;
    }

    // update Front, Right and Up Vectors using the updated Euler angles
    updateCameraVectors();
  }

  // processes input received from a mouse scroll-wheel event. Only requires
  // input on the vertical wheel-axis
  void ProcessMouseScroll(float yoffset) {
    // Zoom -= (float) yoffset;
    // if (Zoom < 1.0f)
    //     Zoom = 1.0f;
    // if (Zoom > 45.0f)
    //     Zoom = 45.0f;

    this->Position = this->Position - this->Front * yoffset;
  }

private:
  // calculates the front vector from the Camera's (updated) Euler Angles
  void updateCameraVectors() {
    if (this->inputType == FPS) {
      // calculate the new Front vector
      glm::vec3 front;
      front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
      front.y = sin(glm::radians(Pitch));
      front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
      Front = glm::normalize(front);
      // also re-calculate the Right and Up vector
      Right = glm::normalize(
          glm::cross(Front,
                     WorldUp)); // normalize the vectors, because their length
                                // gets closer to 0 the more you look up or
                                // down which results in slower movement.
      Up = glm::normalize(glm::cross(Right, Front));
    } else if (this->inputType == ARCBALL) {
      this->Front =
          glm::normalize(this->arcballInput.ArcballTarget - this->Position);
      this->Right = glm::normalize(glm::cross(Front, WorldUp));
      this->Up = glm::normalize(glm::cross(Right, Front));
    }
  }

public:
  void mouse_button_callback(int button, int action, int mods) override;
  void scroll_callback(double x_offset, double y_offset) override;
  void cursor_pos_callback(double xpos, double ypos, double xoffset,
                           double yoffset) override;
  void framebuffer_size_callback(int width, int height) override;
  void key_callback(int key, int scancode, int action, int mods) override;
};
} // namespace ale
#endif
