//
// Created by Alether on 6/17/2025.
//
module;

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <optional>
#include <rapidjson/document.h>
#include <string>

export module common;
using namespace std;
using namespace glm;

export namespace ale::serde {

template<typename T>
T read_vec(const rapidjson::Value &parent, const std::string &name) {
  static_assert(std::is_class<T>::value && T::length() > 0,
                "T must be a glm vector type");

  if (!parent.HasMember(name.c_str())) {
    return T(0.0f);
  }

  const rapidjson::Value &arr = parent[name.c_str()];
  if (!arr.IsArray()) {
    return T(0.0f);
  }

  T val = T(0.0f);
  for (int i = 0; i < arr.Size(); ++i) {
    val[i] = arr[i].GetFloat();
  }
  return val;
}

template<typename VecType>
void write_vec(rapidjson::Value &obj, const std::string &key,
               const VecType &vec,
               rapidjson::Document::AllocatorType &allocator) {
  rapidjson::Value arr(rapidjson::kArrayType);

  const float *data = glm::value_ptr(vec);
  for (int i = 0; i < VecType::length(); ++i) {
    arr.PushBack(data[i], allocator);
  }

  obj.AddMember(rapidjson::Value(key.c_str(), allocator), arr, allocator);
}


} // namespace ale::serde
