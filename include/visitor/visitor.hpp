#pragma once

#include "forward_decls.hpp"

namespace paracl {

class base_visitor {
 public:
  virtual ~base_visitor() {};

  virtual void visit(ast::statement_block *stm)  = 0;
  virtual void visit(ast::calc_expression *stm)  = 0;
  virtual void visit(ast::logic_expression *stm) = 0;
  virtual void visit(ast::un_operator *stm)      = 0;
  virtual void visit(ast::number *stm)           = 0;
  virtual void visit(ast::variable *stm)         = 0;
  virtual void visit(ast::assignment *stm)       = 0;
  virtual void visit(ast::if_operator *stm)      = 0;
  virtual void visit(ast::while_operator *stm)   = 0;
  virtual void visit(ast::read_expression *stm)  = 0;
  virtual void visit(ast::print_function *stm)   = 0;
};

class visitor: public base_visitor {
 public:
  int get_value() const noexcept {
    return curr_value_;
  }

  void set_value(int val) noexcept {
    curr_value_ = val;
  }

 protected:
  int curr_value_ = 0;
};

} // <--- namespace paracl

