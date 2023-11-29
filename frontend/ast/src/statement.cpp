#include "statement.hpp"

namespace frontend {

namespace ast {

void statement::accept(base_visitor *b_visitor) {
  b_visitor->visit(this);
}

void statement_block::accept(base_visitor *b_visitor) {
  b_visitor->visit(this);
}

statement_block::stmts_store &statement_block::statements() {
  return statements_;
}

} // <--- namespace ast

} // <--- namespace frontend
