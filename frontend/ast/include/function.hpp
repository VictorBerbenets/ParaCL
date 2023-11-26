#pragma once

#include <concepts>
#include <string>

#include "statement.hpp"
#include "expression.hpp"

namespace frontend {

namespace ast {

template <typename T>
concept variable_form = std::integral<T> ||
                   std::convertible_to<T, std::string>;

class function: public statement {

};

template <variable_form Var>
class print_function: public function {
  using value_type = Var; 
 public:
  print_function(const value_type& val)
    : var_ {val} {}

  print_function(value_type&& val)
    : var_ {std::move(val)} {}

 private:
  value_type var_;
};

class scan_function: public function {

};

} // <--- namespace ast

} // <--- namespace frontend
