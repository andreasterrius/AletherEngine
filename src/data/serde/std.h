//
// Created by Alether on 1/21/2025.
//

#ifndef OPTIONAL_H
#define OPTIONAL_H

#include <nlohmann/json.hpp>
#include <optional>
//
// namespace nlohmann {
// template <typename T> struct adl_serializer<std::optional<T>> {
//   // Convert std::optional<T> to JSON
//   static void to_json(json &j, const std::optional<T> &opt) {
//     if (opt.has_value()) {
//       j = *opt;
//     } else {
//       j = nullptr;
//     }
//   }
//
//   // Convert JSON to std::optional<T>
//   static void from_json(const json &j, std::optional<T> &opt) {
//     if (j.is_null()) {
//       opt.reset();
//     } else {
//       opt = j.get<T>();
//     }
//   }
// };
// } // namespace nlohmann
#endif // OPTIONAL_H
