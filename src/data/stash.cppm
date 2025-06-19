module;

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

export module data:stash;

export namespace ale::data {

template<typename T>
class Stash {
private:
  std::unordered_map<std::string, std::shared_ptr<T>> data;

public:
  void add(std::string name, std::shared_ptr<T> resource) {
    data[name] = resource;
  }

  std::shared_ptr<T> get_or(std::string name,
                            std::function<T(std::string &name)> func) {
    auto resource = get(name);
    if (resource == nullptr) {
      resource = std::make_shared<T>(func(name));
      add(name, resource);
    }
    return resource;
  }

  std::shared_ptr<T>
  get_or(std::string name,
         std::function<std::shared_ptr<T>(std::string &name)> func) {
    auto resource = get(name);
    if (resource == nullptr) {
      resource = func(name);
      add(name, resource);
    }
    return resource;
  }

  std::shared_ptr<T> get(std::string name) {
    if (data.find(name) == data.end()) {
      return nullptr;
    }
    return data[name];
  }
};
} // namespace ale::data
