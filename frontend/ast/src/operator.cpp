
#include "operator.hpp"
#include "visitor.hpp"

namespace frontend {

namespace ast {

ctrl_statement::ctrl_statement(expression* cond,
                              statement_block* body)
   : condition_ {cond},
     body_ {body} {}

void if_operator::accept(base_visitor *b_visitor) {
  b_visitor->visit(this);
}

void while_operator::accept(base_visitor *b_visitor) {
  b_visitor->visit(this);
}

} // <--- namespace ast

} // <--- namespace frontend