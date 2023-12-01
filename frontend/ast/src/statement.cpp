#include "statement.hpp"

namespace frontend {

namespace ast {

statement::statement(statement_block *parent) noexcept
    : parent_ {parent} {}

void statement::accept(base_visitor *b_visitor) {
  b_visitor->visit(this);
}

void statement::set_parent(statement_block *parent) noexcept {
  parent_ = parent;
}

statement_block::statement_block(statement_block *parent)
    : statement {parent} {}

void statement_block::accept(base_visitor *b_visitor) {
  b_visitor->visit(this);
}

statement_block::stmts_store &statement_block::statements() {
  return statements_;
}

void statement_block::add(statement *stm)  {
  stm->set_parent(this);
  statements_.push_back(stm);
}

} // <--- namespace ast

} // <--- namespace frontend
