//
// Created by Alether on 4/6/2025.
//
module;

#include "editor_root_layout.h"

#include <deque>
#include <variant>
#include <vector>

export module history_stack;

export namespace ale::editor {
using History = std::variant<EditorRootLayout::TransformChangeHistoryCmd>;

class HistoryStack {
private:
  std::deque<History> histories;
  int maximum_history_size;

  std::deque<History> to_be_deleted;

public:
  HistoryStack() : maximum_history_size(100) {};

  HistoryStack(int maximum_history_size)
      : maximum_history_size(maximum_history_size) {}

  void add(History history) {
    histories.push_back(history);
    if (histories.size() > maximum_history_size) {
      histories.pop_front();
    }
    if (!to_be_deleted.empty()) {
      to_be_deleted.clear();
    }
  }

  optional<History> undo() {
    if (histories.empty()) {
      return nullopt;
    }
    auto latest_history = histories.back();
    to_be_deleted.push_back(latest_history);
    histories.pop_back();
    return latest_history;
  }
  optional<History> redo() {
    if (to_be_deleted.empty()) {
      return nullopt;
    }
    auto latest_redo = to_be_deleted.front();
    to_be_deleted.pop_front();
    histories.push_back(latest_redo);
    return latest_redo;
  }
};
} // namespace ale::editor