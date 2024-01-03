#pragma once

#include "expression.hpp"
#include "visitor.hpp"
#include "location.hh"

namespace frontend {

namespace ast {

class ctrl_statement: public statement {
 public:
   ctrl_statement(expression *cond, statement_block *body, yy::location loc)
      : statement {loc},
        condition_ {cond},
        body_ {body} {}

  ~ctrl_statement() override = default;

  expression *condition() const noexcept {
    return condition_;
  }

  statement_block *body() const noexcept {
    return body_;
  }

  void accept_body(base_visitor *b_visitor) {
    body_->accept(b_visitor);
  }

  void accept_condition(base_visitor *b_visitor) {
    condition_->accept(b_visitor);
  }

 protected:
  expression *condition_;
  statement_block *body_;
};

class while_operator: public ctrl_statement {
 public:
  using ctrl_statement::ctrl_statement;

  void accept(base_visitor *b_visitor) override {
    b_visitor->visit(this);
  }
};

class if_operator: public ctrl_statement {
 public:
  using ctrl_statement::ctrl_statement;

  void accept(base_visitor *b_visitor) override {
    b_visitor->visit(this);
  }
};

} // <--- namespace ast

} // <--- namespace frontend
