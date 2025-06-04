//
// Created by Alether on 2/20/2025.
//
module;

#include <utility>
#include <set>

export module window_event;

export namespace ale::input_handling {

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
  void add_listener(WindowEventListener *listener) {
    this->listeners.insert(listener);
  }
  void remove_listener(WindowEventListener *listener) {
    this->listeners.erase(listener);
  }
  std::set<WindowEventListener *> &get_listeners() {
    return this->listeners;
  }
};
}; // namespace ale