#pragma once

#include <vector>
#include <utility>

#include "expression.hpp"
#include "location.hh"

namespace frontend {

namespace ast {

class object_type: public expression {
 public:
  using size_type = std::size_t;

  void accept(base_visitor*) override {}

  using expression::expression;
};

class integer_literal: public object_type {
  using value_type = int;
 public:
  integer_literal(value_type num, yy::location loc)
      : object_type {loc},
        value_ {num} {}

  const value_type &get_value() const noexcept {
    return value_;
  }
/*
  void accept(base_visitor *b_visitor) override {
    b_visitor->visit(this);
  }
*/
 private:
  value_type value_;
};

class integer_variable: public object_type {

};

// if we know the size before running
class array: public object_type {
 public:
  array(size_type sz, int init_val = {0})
      : stor_ (sz, init_val) {}
 protected:
  std::vector<int> stor_;
};

class array_elem: public object_type {
 public:
  array_elem(std::string name, std::vector<expression*> indexes,
             yy::location loc)
      : object_type {loc},
        arr_name_ {std::move(name)},
        indexes_  {std::move(indexes)} {}
  
  void accept(base_visitor *b_visitor) override {
    b_visitor->visit(this);
  }

  auto indexes() const { return indexes_;  }
  std::string name() const { return arr_name_; }
 private:
  std::string arr_name_;
  std::vector<expression*> indexes_;
};

class init_array: public array {

};

// if we know the size before running

class dynamic_array: public array {

};

//class multi_array: public 
  
} // <--- namespace ast

} // <--- namespace frontend

