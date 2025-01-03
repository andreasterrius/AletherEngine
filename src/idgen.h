#ifndef IDGEN_H
#define IDGEN_H

#include <stdint.h>

#include <string>
#include <typeinfo>
#include <unordered_map>

namespace ale {
class IdGen {
 private:
  // unordered_map<std::string, uint64_t> id_storage;

 public:
  // template <typename T>
  // uint64_t next() {
  //   auto type_id = std::typeid<T>.name();
  //   if (id_storage.find(type_id) == id_storage.end()) {
  //     id_storage[type_id] = 0;
  //   }
  //   return id_storage[type_id]++;
  // }
};
}  // namespace ale

#endif