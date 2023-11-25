#pragma once

#include <memory>
#include <vector>
#include <concepts>
#include <type_traits>


#include "statement.hpp"

namespace frontend {

namespace ast {

template <typename T>
concept derived_from = std::derived_from<T, statement>;

class ast final {
  public:

  private:
    std::unique_ptr<statement> root_;
};

class qualifier: public statement {

};

template <derived_from NodeType>
auto make_node(const auto type,
               const auto left,
               const auto right) requires (std::is_enum_v<decltype(type)>) {
  auto ret_ptr = std::make_unique<NodeType>(type, left, right);
  return ret_ptr;
}

template <derived_from NodeType>
auto make_node(const auto type,
              const auto child) requires (std::is_enum_v<decltype(type)>) { 
  auto ret_ptr = std::make_unique<NodeType>(type, child);
  return ret_ptr;
}


} // <--- namespace ast

} // <--- namespace frontend

