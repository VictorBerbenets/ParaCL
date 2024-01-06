#pragma once

#include <vector>

#include "expression.hpp"
#include "visitor.hpp"
#include "location.hh"

namespace frontend {

namespace ast {

class ctrl_statement: public statement {
 public:
   ctrl_statement(expression *cond, statement *body, yy::location loc)
      : statement {loc},
        condition_ {cond},
        body_ {body} {}

  expression *condition() noexcept {
    return condition_;
  }

  statement *body() noexcept {
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
  statement *body_;
};

class while_operator: public ctrl_statement {
 public:
  using ctrl_statement::ctrl_statement;

  void accept(base_visitor *b_visitor) override {
    b_visitor->visit(this);
  }
};

class if_operator final: public ctrl_statement {
 public:
  using ctrl_statement::ctrl_statement;
 
  if_operator(expression *cond, statement *body, statement *else_block,
              yy::location loc)
      : ctrl_statement {cond, body, loc},
        else_block_ {else_block} {}

  void accept(base_visitor *b_visitor) override {
    b_visitor->visit(this);
  }

  void accept_else(base_visitor *b_visitor) {
    else_block_->accept(b_visitor);
  }

  statement *else_block() noexcept { return else_block_; }

 private:
  statement *else_block_ {nullptr};
};



} // <--- namespace ast

} // <--- namespace frontend
