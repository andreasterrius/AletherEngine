//
// Created by Alether on 3/2/2025.
//

export module operation;

#include "transform.h"

export namespace ale::operation {
class Operation { // interface that has undo and redo
public:
  virtual ~Operation() = default;
  virtual auto undo() = 0;
  virtual auto redo() = 0;
};

class ObjectMoveOperation : public Operation {
public:
  Transform before = Transform{};
  Transform after = Transform{};

public:
  explicit ObjectMoveOperation(Transform before) : before(before) {}
  auto end_operation(const Transform &after) { this->after = after; }
  auto undo() override {}
  auto redo() override {};
};

} // namespace ale::operation