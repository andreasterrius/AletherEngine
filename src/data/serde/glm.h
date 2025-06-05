//
// Created by Alether on 1/20/2025.
//

#ifndef ADL_SERIALIZER_H
#define ADL_SERIALIZER_H

#include <glm/detail/type_quat.hpp>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
//
// // partial specialization (full specialization works too)
// namespace nlohmann {
// template <> struct adl_serializer<glm::vec3> {
//   static void to_json(json &j, const glm::vec3 &v) { j = {v.x, v.y, v.z}; }
//
//   static void from_json(const json &j, glm::vec3 &v) {
//     v.x = j[0].get<float>();
//     v.y = j[1].get<float>();
//     v.z = j[2].get<float>();
//   }
// };
//
// template <> struct adl_serializer<glm::quat> {
//   static void to_json(json &j, const glm::quat &v) { j = {v.w, v.x, v.y,
//   v.z}; }
//
//   static void from_json(const json &j, glm::quat &v) {
//     v.w = j[0].get<float>();
//     v.x = j[1].get<float>();
//     v.y = j[2].get<float>();
//     v.z = j[3].get<float>();
//   }
// };
// } // namespace nlohmann
#endif // ADL_SERIALIZER_H
