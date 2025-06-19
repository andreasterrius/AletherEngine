module;

#include <entt/entt.hpp>

export module editor:history.history;
import data;

using namespace ale::data;

export namespace ale::editor::history {
class History {
public:
  virtual ~History() = default;
  virtual void execute(entt::registry &world) = 0;
  virtual void undo(entt::registry &world) = 0;
};

class TransformHistory : public History {
public:
  entt::entity entity;
  Transform before;
  Transform after;

  TransformHistory(entt::entity entity, Transform before, Transform after) :
      entity(entity),
      before(before),
      after(after) {}

  void execute(entt::registry &world) override {
    auto transform = world.try_get<Transform>(entity);
    if (transform == nullptr) {
      return;
    }
    *transform = after;
  }
  void undo(entt::registry &world) override {
    auto transform = world.try_get<Transform>(entity);
    if (transform == nullptr) {
      return;
    }
    *transform = before;
  }
};

}; // namespace ale::editor::history
