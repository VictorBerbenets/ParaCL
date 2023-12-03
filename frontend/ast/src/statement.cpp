#include "statement.hpp"

namespace frontend {

namespace ast {

statement::statement(statement_block *parent) noexcept
    : parent_ {parent} {}

void statement::set_parent(statement_block *parent) noexcept {
  parent_ = parent;
}

statement_block::statement_block(statement_block *parent)
    : statement {parent} {}

void statement_block::accept(base_visitor *b_visitor) {
  b_visitor->visit(this);
}

statement_block::ScopeIter statement_block::begin() noexcept {
  return statements_.begin();
}

statement_block::ScopeIter statement_block::end() noexcept {
  return statements_.end();
}

statement_block::ConstScopeIter statement_block::cbeing() const noexcept {
  return statements_.cbegin();
}

statement_block::ConstScopeIter statement_block::cend() const noexcept {
  return statements_.cend();
}

void statement_block::add(statement *stm)  {
  stm->set_parent(this);
  statements_.push_back(stm);
}

} // <--- namespace ast

} // <--- namespace frontend
