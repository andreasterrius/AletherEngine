module;

export module operation;

import transform;

export namespace ale::data {

class Operation { // interface that has undo and redo
public:
  virtual ~Operation() = default;
  virtual void undo() = 0;
  virtual void redo() = 0;
};

class ObjectMoveOperation : public Operation {
public:
  Transform before = Transform{};
  Transform after = Transform{};

public:
  explicit ObjectMoveOperation(Transform before) : before(before) {}
  auto end_operation(const Transform &after) { this->after = after; }
  void undo() override {}
  void redo() override {};
};

} // namespace ale::data
