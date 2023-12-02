#pragma once

#include "expression.hpp"
#include "visitor.hpp"

namespace frontend {

namespace ast {

class ctrl_statement: public statement {
 public:
  ~ctrl_statement() override = default;

  expression *condition() const noexcept {
    return condition_;
  }
  statement_block *body() const noexcept {
    return body_;
  }

 protected:
  ctrl_statement(expression *cond, statement_block *body);

  expression *condition_;
  statement_block *body_;
};

class while_operator: public ctrl_statement {
 public:
  while_operator(expression *cond, statement_block *body)
    : ctrl_statement(cond, body) {}
  void accept(base_visitor *b_visitor) override;

};

class if_operator: public ctrl_statement {
 public:
   if_operator(expression *cond, statement_block *body)
    : ctrl_statement(cond, body) {}
  void accept(base_visitor *b_visitor) override;
};

} // <--- namespace ast

} // <--- namespace frontend
