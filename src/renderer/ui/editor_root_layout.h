//
// Created by Alether on 1/10/2025.
//

#ifndef ROOT_LAYOUT_H
#define ROOT_LAYOUT_H
#include "src/window.h"

#include <complex.h>

namespace ale::ui {
class EditorRootLayout {
public:
  struct Event {
    bool is_exit_clicked = false;
  };

public:
  Event start(ivec2 pos, ivec2 size);

  void end();
};
} // namespace ale::ui

#endif // ROOT_LAYOUT_H
