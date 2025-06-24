//
// Created by Alether on 6/17/2025.
//
module;

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <optional>
#include <rfl.hpp>
#include <string>
#include <vector>

export module serde:common;
import data;
import graphics;

using namespace std;
using namespace glm;
using namespace ale::data;
using namespace ale::graphics;

namespace rfl {

template<>
struct Reflector<vec2> {
  using ReflType = std::vector<float>;

  static vec2 to(const ReflType &str) noexcept {
    return vec2(str.at(0), str.at(1));
  }

  static ReflType from(const vec2 &v) { return vector{v[0], v[1]}; }
};

template<>
struct Reflector<vec3> {
  using ReflType = std::vector<float>;

  static vec3 to(const ReflType &str) noexcept {
    return vec3(str.at(0), str.at(1), str.at(2));
  }

  static ReflType from(const vec3 &v) { return vector{v[0], v[1], v[2]}; }
};

template<>
struct Reflector<vec4> {
  using ReflType = std::vector<float>;

  static vec4 to(const ReflType &str) noexcept {
    return vec4(str.at(0), str.at(1), str.at(2), str.at(3));
  }

  static ReflType from(const vec4 &v) { return vector{v[0], v[1], v[2], v[3]}; }
};

template<>
struct Reflector<quat> {
  using ReflType = std::vector<float>;

  static quat to(const ReflType &str) noexcept {
    return quat(str.at(0), str.at(1), str.at(2), str.at(3));
  }

  static ReflType from(const quat &v) { return vector{v[0], v[1], v[2], v[3]}; }
};

} // namespace rfl
//
// export namespace ale::serde {
//
// // Primitives
// void write_string(rapidjson::Value &parent_json, const std::string &key,
//                   const string &val,
//                   rapidjson::Document::AllocatorType &allocator) {
//   rapidjson::Value val_json;
//   val_json.SetString(val.c_str(), val.size(), allocator);
//   parent_json.AddMember(rapidjson::Value(key.c_str(), allocator), val_json,
//                         allocator);
// }
//
// void write_float(rapidjson::Value &parent_json, const std::string &key,
//                  const float val,
//                  rapidjson::Document::AllocatorType &allocator) {
//   rapidjson::Value val_json;
//   val_json.SetFloat(val);
//   parent_json.AddMember(rapidjson::Value(key.c_str(), allocator), val_json,
//                         allocator);
// }
//
// // Vec
// template<typename T>
// T read_vec(const rapidjson::Value &parent_json, const std::string &name) {
//   static_assert(std::is_class<T>::value && T::length() > 0,
//                 "T must be a glm vector type");
//
//   if (!parent_json.HasMember(name.c_str())) {
//     return T(0.0f);
//   }
//
//   const rapidjson::Value &arr = parent_json[name.c_str()];
//   if (!arr.IsArray()) {
//     return T(0.0f);
//   }
//
//   T val = T(0.0f);
//   for (int i = 0; i < arr.Size(); ++i) {
//     val[i] = arr[i].GetFloat();
//   }
//   return val;
// }
//
// template<typename VecType>
// void write_vec(rapidjson::Value &parent_json, const std::string &key,
//                const VecType &vec,
//                rapidjson::Document::AllocatorType &allocator) {
//   rapidjson::Value arr(rapidjson::kArrayType);
//
//   const float *data = glm::value_ptr(vec);
//   for (int i = 0; i < VecType::length(); ++i) {
//     arr.PushBack(data[i], allocator);
//   }
//
//   parent_json.AddMember(rapidjson::Value(key.c_str(), allocator), arr,
//                         allocator);
// }
//
// // Transform
// void write_transform(rapidjson::Value &parent_json, const std::string &key,
//                      const Transform &transform,
//                      rapidjson::Document::AllocatorType &allocator) {
//   rapidjson::Value transform_obj(rapidjson::kObjectType);
//   write_vec<vec3>(transform_obj, "translation", transform.translation,
//                   allocator);
//   write_vec<vec3>(transform_obj, "scale", transform.scale, allocator);
//   write_vec<quat>(transform_obj, "rotation", transform.rotation, allocator);
//
//   parent_json.AddMember(rapidjson::Value(key.c_str(), allocator),
//   transform_obj,
//                         allocator);
// }
//
// // Light
// void write_light(rapidjson::Value &parent_json, const std::string &key,
//                  const Light &light,
//                  rapidjson::Document::AllocatorType &allocator) {
//
//   rapidjson::Value obj(rapidjson::kObjectType);
//   write_vec<vec3>(obj, "attenuation", light.attenuation, allocator);
//   write_vec<vec3>(obj, "color", light.color, allocator);
//   write_float(obj, "radius", light.radius, allocator);
//
//   parent_json.AddMember(rapidjson::Value(key.c_str(), allocator), obj,
//                         allocator);
// }
// void write_ambient_light(rapidjson::Value &parent_json, const std::string
// &key,
//                          const AmbientLight &light,
//                          rapidjson::Document::AllocatorType &allocator) {
//
//   rapidjson::Value obj(rapidjson::kObjectType);
//   write_float(obj, "intensity", light.intensity, allocator);
//   write_vec<vec3>(obj, "background_color", light.background_color,
//   allocator); write_vec<vec3>(obj, "color", light.color, allocator);
//
//   parent_json.AddMember(rapidjson::Value(key.c_str(), allocator), obj,
//                         allocator);
// }
//
// void write_scene_node(rapidjson::Value &parent_json, const std::string &key,
//                       const SceneNode &scene_node,
//                       rapidjson::Document::AllocatorType &allocator) {
//
//   rapidjson::Value obj(rapidjson::kObjectType);
//   write_string(obj, "name", scene_node.name, allocator);
//
//   parent_json.AddMember(rapidjson::Value(key.c_str(), allocator), obj,
//                         allocator);
// }
//
// } // namespace ale::serde
