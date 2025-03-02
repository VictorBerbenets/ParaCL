#pragma once

#include <vector>

#include "expression.hpp"
#include "location.hh"
#include "visitor.hpp"

namespace paracl {

namespace ast {

class ctrl_statement : public statement {
public:
  ctrl_statement(expression *cond, statement *body, yy::location loc)
      : statement{loc}, condition_{cond}, body_{body} {}

  expression *condition() noexcept { return condition_; }

  statement *body() noexcept { return body_; }

protected:
  expression *condition_;
  statement *body_;
};

class while_operator : public ctrl_statement {
public:
  using ctrl_statement::ctrl_statement;

  void accept(VisitorBase *Vis) override { Vis->visit(this); }

  void accept(CodeGenVisitor *CodeGenVis) override { CodeGenVis->visit(this); }
};

class if_operator final : public ctrl_statement {
public:
  using ctrl_statement::ctrl_statement;

  if_operator(expression *cond, statement *body, statement *else_block,
              yy::location loc)
      : ctrl_statement{cond, body, loc}, else_block_{else_block} {}

  void accept(VisitorBase *Vis) override { Vis->visit(this); }

  void accept(CodeGenVisitor *CodeGenVis) override { CodeGenVis->visit(this); }

  statement *else_block() noexcept { return else_block_; }

private:
  statement *else_block_{nullptr};
};

} // namespace ast

} // namespace paracl
