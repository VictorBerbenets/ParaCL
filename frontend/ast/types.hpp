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

  void accept(base_visitor *b_visitor) override {
    b_visitor->visit(this);
  }

 private:
  value_type value_;
};

// if we know the size before running
class array: public object_type {
 public:
  array(statement_block *scope, expression *e1, expression *e2, yy::location loc){}
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

template <typename T>
class assignment;

template <>
class assignment<int>: public expression {
 public:
  assignment(statement_block *curr_block, const std::string &name,
             expression *expr, yy::location loc)
    : expression {curr_block, loc},
      name_ {name},
      identifier_ {expr} {
    parent_->declare(name_);
  }

  assignment(statement_block *curr_block, std::string &&name,
             expression *expr, yy::location loc)
      : expression {curr_block, loc},
        name_ {std::move(name)},
        identifier_ {expr} {
    parent_->declare(name_);
  }

  void accept(base_visitor *base_visitor) override {
    base_visitor->visit(this);
  }

  expression *ident_exp() noexcept {
    return identifier_;
  }

  void redefine(int value) {
    parent_->redefine(name_, value);
  }

  const std::string &name() const noexcept {
    return name_;
  }
 private:
  std::string name_;
  expression* identifier_;
};

//class multi_array: public 
  
} // <--- namespace ast

} // <--- namespace frontend

