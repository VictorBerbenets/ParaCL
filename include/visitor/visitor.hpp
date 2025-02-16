#pragma once

#include "forward_decls.hpp"

namespace paracl {

class base_visitor {
 public:
  virtual ~base_visitor() {};

  virtual void visit_interpret(ast::statement_block *stm)  = 0;
  virtual void visit_interpret(ast::calc_expression *stm)  = 0;
  virtual void visit_interpret(ast::logic_expression *stm) = 0;
  virtual void visit_interpret(ast::un_operator *stm)      = 0;
  virtual void visit_interpret(ast::number *stm)           = 0;
  virtual void visit_interpret(ast::variable *stm)         = 0;
  virtual void visit_interpret(ast::assignment *stm)       = 0;
  virtual void visit_interpret(ast::if_operator *stm)      = 0;
  virtual void visit_interpret(ast::while_operator *stm)   = 0;
  virtual void visit_interpret(ast::read_expression *stm)  = 0;
  virtual void visit_interpret(ast::print_function *stm)   = 0;
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

