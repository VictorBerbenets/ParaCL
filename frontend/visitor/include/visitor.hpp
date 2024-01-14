#pragma once

#include "forward_decls.hpp"

namespace frontend {

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
  int accept(ast::statement* node);
    //node->accept(this);
    //return curr_value_;
 // }

 protected:
  int curr_value_ = 0;
};

} // <--- namespace frontend

