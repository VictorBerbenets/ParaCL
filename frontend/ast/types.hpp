#pragma once

#include <vector>
#include <utility>

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
  array(std::string name, statement_block *scope,
        expression *, expression *, yy::location loc)
            : object_type {name, scope, loc} {}

  void accept(base_visitor *b_visitor) override {
    b_visitor->visit(this); 
  }

 protected:
  std::vector<int> stor_;
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
#if 0
  void declare() {
    scope()->declare(this);
  }
#endif
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
 
  value_type get_value() override {
    //auto right_scope = scope()->find(name());
    //auto array_ptr   = static_cast<array*>(right_scope->object(name()));
   return 0; 

  }

  void set_value(value_type val) override {

  }

  auto indexes() const { return indexes_;  }
 private:
  std::vector<expression*> indexes_;
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

