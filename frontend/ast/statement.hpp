#pragma once

#include "visitor.hpp"
#include "location.hh"

namespace frontend {

namespace ast {

class statement_block;

class statement {
 public:
  virtual ~statement() = default;

  virtual void accept(base_visitor *b_visitor) = 0;

  void set_parent(statement_block *parent) noexcept {
    parent_ = parent;
  }

  statement_block *scope() noexcept {
    return parent_;
  }

  yy::location location() const { return loc_; }

 protected:
  explicit statement(yy::location loc)
    : loc_ {loc} {}

  statement(statement_block *parent, yy::location loc = yy::location{})
    : parent_ {parent},
      loc_ {loc} {}

  statement() = default;

  statement_block *parent_;
  yy::location loc_;
};

} // <--- namespace ast

} // <--- namespace frontend

