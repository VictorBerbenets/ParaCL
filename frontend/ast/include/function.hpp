#pragma once

#include <concepts>
#include <string>

#include "statement.hpp"
#include "expression.hpp"

namespace frontend {

namespace ast {

template <typename T>
concept variable_form = std::integral<T> ||
                        std::constructible_from<std::string, T>;

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
  using value_type = int;
 public:
  scan_function(const std::string& var_name);
  scan_function(std::string&& var_name);

 private:
  std::string var_name_;
  value_type value_;

};

} // <--- namespace ast

} // <--- namespace frontend
