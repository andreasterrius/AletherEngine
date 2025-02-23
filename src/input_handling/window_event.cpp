//
// Created by Alether on 2/20/2025.
//

#include "window_event.h"

#include <utility>

namespace ale {

void WindowEventProducer::add_listener(WindowEventListener *listener) {
  this->listeners.insert(listener);
}

void WindowEventProducer::remove_listener(WindowEventListener *listener) {
  this->listeners.erase(listener);
}
std::set<WindowEventListener *> &WindowEventProducer::get_listeners() {
  return this->listeners;
}

}; // namespace ale