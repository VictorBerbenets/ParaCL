#pragma once

#include "expression.hpp"
#include "visitor.hpp"

namespace frontend {

namespace ast {

class ctrl_statement: public statement {
 public:
  ~ctrl_statement() override = default;

  void accept(base_visitor *b_visitor) override;

  ctrl_statement(CtrlStatement type, expression *cond, statement_block *body);
//  protected:
  CtrlStatement type_;
  expression *condition_;
  statement_block *body_;
};

class while_operator: public ctrl_statement {
 public:
  void accept(base_visitor *b_visitor) override;

};

class if_operator: public ctrl_statement {
 public:
  void accept(base_visitor *b_visitor) override;
};

} // <--- namespace ast

} // <--- namespace frontend
