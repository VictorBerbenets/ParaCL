#include "expression.hpp"

namespace frontend {

namespace ast {

ctrl_statement::ctrl_statement(CtrlStatement type, expression* cond,
                              statement_block* body)
   : type_ {type},
     condition_ {cond},
     body_ {body} {}

} // <--- namespace ast

} // <--- namespace frontend
