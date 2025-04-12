//
// Created by Alether on 4/6/2025.
//
module;

#include <deque>
#include <optional>
#include <variant>

export module command_stack;

import command;

export namespace ale::editor {

  class HistoryStack {
private:
    // typedef std::variant<editor::EditorRootLayout::TransformChangeHistoryCmd> History;
    using History = editor::Cmd;
    std::deque<History> histories;
    int maximum_history_size;

    std::deque<History> to_be_deleted;

public:
    HistoryStack() : maximum_history_size(100) {};

    HistoryStack(int maximum_history_size) : maximum_history_size(maximum_history_size) {}

    void add(History history) {
      histories.push_back(history);
      if (histories.size() > maximum_history_size) {
        histories.pop_front();
      }
      if (!to_be_deleted.empty()) {
        to_be_deleted.clear();
      }
    }

    std::optional<History> undo() {
      if (histories.empty()) {
        return std::nullopt;
      }
      auto latest_history = histories.back();
      to_be_deleted.push_back(latest_history);
      histories.pop_back();
      return latest_history;
    }
    std::optional<History> redo() {
      if (to_be_deleted.empty()) {
        return std::nullopt;
      }
      auto latest_redo = to_be_deleted.front();
      to_be_deleted.pop_front();
      histories.push_back(latest_redo);
      return latest_redo;
    }
  };
}  // namespace ale::editor
