#pragma once

#include <concepts>
#include <string>
#include <variant>

#include "statement.hpp"
#include "expression.hpp"

namespace frontend {

namespace ast {

// template <typename T>
// concept variable_form = std::integral<T> ||
//                         std::constructible_from<std::string, T>;

class function: public statement {
public:
};

// template <variable_form Var>
class print_function: public function {
 public:
  print_function(int val)
    : var_ {val} {}

  print_function(std::string&& val)
    : var_ {std::move(val)} {}

  print_function(const std::string& val)
    : var_ {val} {}

 private:
  std::variant<int, std::string> var_;
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
