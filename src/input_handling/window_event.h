//
// Created by Alether on 2/20/2025.
//

#ifndef WINDOW_EVENT_LISTENER_H
#define WINDOW_EVENT_LISTENER_H

#include <set>
#include <vector>

namespace ale {

enum WindowEventCallbackType {
  MOUSE_BUTTON,
  CURSOR_POSITION,
  FRAMEBUFFER_SIZE,
  SCROLL,
  KEY
};

class WindowEventListener {
public:
  virtual ~WindowEventListener() = default;
  virtual void mouse_button_callback(int button, int action, int mods) = 0;
  virtual void cursor_pos_callback(double xpos, double ypos, double xoffset,
                                   double yoffset) = 0;
  virtual void framebuffer_size_callback(int width, int height) = 0;
  virtual void scroll_callback(double x_offset, double y_offset) = 0;
  virtual void key_callback(int key, int scancode, int action, int mods) = 0;
};

// Producer HAS to outlive the listeners!
class WindowEventProducer {
protected:
  // non owning pointers
  std::set<WindowEventListener *> listeners;

public:
  std::set<WindowEventListener *> &get_listeners();
  void add_listener(WindowEventListener *listener);
  void remove_listener(WindowEventListener *listener);
};
}; // namespace ale

// class WindowEventListener {
//   // This is a weak pointer
//   WindowEventProducer *producer = nullptr;
//
// public:
//
// };

#endif // WINDOW_EVENT_LISTENER_H
