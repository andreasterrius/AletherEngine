//
// Created by Alether on 4/12/2025.
//
module;

#include <entt/entt.hpp>
#include <variant>
#include "src/data/transform.h"

export module command;

import content_browser;
import item_inspector;

export namespace ale::editor {

struct ExitCmd {};
struct NewWorldCmd {};
struct NewObjectCmd {
  ContentBrowser::Entry new_object;
};
struct TransformChangeNotif {
  entt::entity entity;
  Transform before;
  Transform after;
};
struct UndoCmd {};
struct RedoCmd {};
struct SaveWorldCmd {};
struct LoadWorldCmd {};
struct CameraLookAtEntityCmd {
  entt::entity entity;
};

using Cmd = std::variant<ExitCmd, NewWorldCmd, NewObjectCmd, ItemInspector::Cmd,
                         TransformChangeNotif, UndoCmd, RedoCmd, SaveWorldCmd,
                         LoadWorldCmd, CameraLookAtEntityCmd>;
} // namespace ale::editor
