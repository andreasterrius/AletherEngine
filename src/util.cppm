module;
#include <rfl.hpp>
#include <utility>
#include <variant>

export module util;

template<class... Ts>
struct Overload : Ts... {
  using Ts::operator()...;
};
template<class... Ts>
Overload(Ts...) -> Overload<Ts...>;

export template<typename Variant, typename... Lambdas>
void match(Variant &&var, Lambdas &&...lambdas) {
  std::visit(Overload{std::forward<Lambdas>(lambdas)...},
             std::forward<Variant>(var));
}
