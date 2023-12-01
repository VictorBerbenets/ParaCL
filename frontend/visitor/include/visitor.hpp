#pragma once

#include "forward_decls.hpp"

namespace frontend {

class base_visitor {
 public:
  virtual ~base_visitor() {};

  virtual void visit(ast::statement *stm)        = 0;
  virtual void visit(ast::statement_block *stm)  = 0;
  virtual void visit(ast::expression *stm)       = 0;
  virtual void visit(ast::bin_operator *stm)     = 0;
  virtual void visit(ast::un_operator *stm)      = 0;
  virtual void visit(ast::logic_expression *stm) = 0;
  virtual void visit(ast::number *stm)           = 0;
  virtual void visit(ast::variable *stm)         = 0;
  virtual void visit(ast::assignment *stm)   = 0;
  virtual void visit(ast::ctrl_statement *stm)   = 0;
  virtual void visit(ast::scan_function *stm)    = 0;
  virtual void visit(ast::print_function *stm)   = 0;
};


class visitor: public base_visitor {
 public:
  ~visitor() override = default;

  int accept(ast::statement* node);

 protected:
  int curr_value_;
};

} // <--- namespace frontend

