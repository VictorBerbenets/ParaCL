#pragma once

#include <variant>

#include "ast_includes.hpp"

namespace frontend {

class base_visitor {
 public:
  virtual ~base_visitor() {};

  virtual void visit(statement* stm)        = 0;
  virtual void visit(statement_block* stm)  = 0;
  virtual void visit(expression* stm)       = 0;
  virtual void visit(bin_operator* stm)     = 0;
  virtual void visit(un_operator* stm)      = 0;
  virtual void visit(logic_expression* stm) = 0;
  virtual void visit(number* stm)           = 0;
  virtual void visit(variable* stm)         = 0;
  virtual void visit(ctrl_statement* stm)   = 0;
  virtual void visit(scan_function* stm)    = 0;
  virtual void visit(print_function* stm)   = 0;
};


class visitor: public base_visitor {
 public:
  ~visitor() override = default;

  template <typename T>
  T accept(i_node* node) {
    node->accept(this);
    return curr_value_;
  }

 protected:
  std::variant<int, std::string> curr_value_;
};

} // <--- namespace frontend

