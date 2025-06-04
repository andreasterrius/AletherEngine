//
// Created by Alether on 4/12/2025.
//
module;

#include <entt/entt.hpp>
#include <variant>

export module command;

import content_browser;
import transform;
import item_inspector;

using namespace ale::data;

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
struct SaveWorldCmd {
  // if not provided, a dialog box will be shown
  std::optional<std::string> path;
};
struct LoadWorldCmd {
  // if not provided, a dialog box will be shown
  std::optional<std::string> path;
};
struct CameraLookAtEntityCmd {
  entt::entity entity;
};

using Cmd = std::variant<ExitCmd, NewWorldCmd, NewObjectCmd, ItemInspector::Cmd,
                         TransformChangeNotif, UndoCmd, RedoCmd, SaveWorldCmd,
                         LoadWorldCmd, CameraLookAtEntityCmd>;
} // namespace ale::editor
