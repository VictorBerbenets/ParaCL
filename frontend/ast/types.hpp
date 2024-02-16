#pragma once

#include <vector>
#include <utility>
#include <iterator>

#include "expression.hpp"
#include "location.hh"
#include "base_type.hpp"
#include "statement_block.hpp"

namespace frontend {

namespace ast {

class integer_literal: public expression {
  using value_type = int;
 public:
  integer_literal(value_type num, yy::location loc)
      : expression {loc},
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
  using value_type               = int;
  using size_type                = std::size_t;
  using iterator                 = std::vector<value_type>::iterator; 
  using const_iterator           = std::vector<value_type>::const_iterator; 
  using reverse_iterator         = std::vector<value_type>::reverse_iterator; 
  using const_reverse_iterator   = std::vector<value_type>::const_reverse_iterator;

  array(std::string name, statement_block *scope,
        expression *, expression *, yy::location loc)
            : object_type {name, scope, loc} {}

  void accept(base_visitor *b_visitor) override {
    b_visitor->visit(this); 
  }
  
  auto begin() noexcept { return stor_.begin(); }
  auto end()   noexcept { return stor_.end(); }
  auto cbegin() const noexcept { return stor_.cbegin(); }
  auto cend()   const noexcept { return stor_.cend(); }
  auto rbegin() noexcept { return std::make_reverse_iterator(begin());}
  auto rend()   noexcept { return std::make_reverse_iterator(end()); }
  auto crbegin() const noexcept { return std::make_reverse_iterator(cbegin());}
  auto crend()   const noexcept { return std::make_reverse_iterator(cend()); }

  friend class array_elem;
 protected:
  std::vector<value_type> stor_;
};

class lvalue_variable: public object_type {
 public:
  using value_type = int;

  lvalue_variable(std::string var_name, statement_block *curr_block,
           yy::location l)
      : object_type {var_name, curr_block, l} {}
  
  virtual void set_value(value_type val)  = 0;
  virtual value_type get_value()          = 0;
};

class integer_variable: public lvalue_variable {
 public:
  integer_variable(std::string var_name, statement_block *curr_block,
           yy::location l)
      : lvalue_variable {var_name, curr_block, l} {}

  void accept(base_visitor *b_visitor) override {
    b_visitor->visit(this);
  }

  value_type get_value() override { return value_; }
  void set_value(value_type val) override { value_ = val; }

 private:
  value_type value_;
};

class array_elem: public lvalue_variable {
 public:
  array_elem(std::string name, statement_block *scope,
             std::vector<expression*> indexes, yy::location loc)
      : lvalue_variable {name, scope, loc},
        indexes_  {std::move(indexes)} {}
 
  void accept(base_visitor *b_visitor) override {
    b_visitor->visit(this);
  }
 
  void set_index(size_type index) noexcept { index_ = index; }
 
  value_type get_value() override {
    return get_array()->stor_[index_];
  }

  void set_value(value_type val) override {
    get_array()->stor_[index_] = val; 
  }

  const auto &indexes() const { return indexes_; }
  
  array *get_array() {
    auto arr_scope = scope()->find(name_);
    return static_cast<array*>(arr_scope->object(name_));
  }

 private:
  std::vector<expression*> indexes_;
  size_type index_;
};

class init_array: public array {
  void accept(base_visitor *b_visitor) override {
    b_visitor->visit(this);
  }
};

// if we know the size before running

class dynamic_array: public array {
  void accept(base_visitor *b_visitor) override {
    b_visitor->visit(this);
  }

};

//class multi_array: public 
  
} // <--- namespace ast

} // <--- namespace frontend

