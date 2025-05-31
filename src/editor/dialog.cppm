//
// Created by Alether on 4/19/2025.
//
#include <nfd.hpp>
#include <optional>
#include <string>

export module dialog;

export namespace ale::editor {

std::optional<std::string> save_file_picker() {
  NFD::UniquePath out_path;
  nfdresult_t result = NFD::SaveDialog(out_path, nullptr);

  if (result == NFD_OKAY) {
    return std::make_optional(out_path.get());
  }
  return std::nullopt;
}

std::optional<std::string> open_file_picker() {
  NFD::UniquePath out_path;
  nfdresult_t result = NFD::OpenDialog(out_path, nullptr);

  if (result == NFD_OKAY) {
    return std::make_optional(out_path.get());
  }
  return std::nullopt;
}


} // namespace ale::editor
