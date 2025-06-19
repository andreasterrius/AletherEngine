//
// Created by Alether on 4/6/2025.
//
module;

#include <deque>
#include <entt/entt.hpp>
#include <memory>
#include <optional>

export module editor:history.history_stack;
import :history.history;

using namespace std;

export namespace ale::editor::history {

class HistoryStack {
private:
  // typedef std::variant<editor::EditorRootLayout::TransformChangeHistoryCmd>
  // History;
  int maximum_history_size;
  deque<unique_ptr<History>> histories; // contains undo command
  deque<unique_ptr<History>> to_be_deleted; // contains redo command

public:
  HistoryStack() : maximum_history_size(100) {};

  HistoryStack(int maximum_history_size) :
      maximum_history_size(maximum_history_size) {}

  void add(unique_ptr<History> history) {
    histories.push_back(std::move(history));
    if (histories.size() > maximum_history_size) {
      histories.pop_front();
    }
    if (!to_be_deleted.empty()) {
      to_be_deleted.clear();
    }
  }

  void undo(entt::registry &world) {
    if (histories.empty()) {
      return;
    }

    auto latest_history = std::move(histories.back());
    histories.pop_back();

    latest_history->undo(world);

    to_be_deleted.push_back(std::move(latest_history));
  }

  void redo(entt::registry &world) {
    if (to_be_deleted.empty()) {
      return;
    }
    auto latest_redo = std::move(to_be_deleted.front());
    to_be_deleted.pop_front();

    latest_redo->execute(world);

    histories.push_back(std::move(latest_redo));
  }
};
} // namespace ale::editor::history
