#include "expression.hpp"

namespace frontend {

namespace ast {
logic_expression::logic_expression(LogicOp type, expression *left, expression *right)
    : type_ {type},
      left_ {left},
      right_ {right} {}
int logic_expression::eval() const {
  /* declare soon */
  return {};
}

bin_operator::bin_operator(BinOp type, pointer_type left, pointer_type right)
    : type_  {type},
      left_  {left},
      right_ {right} {}

int bin_operator::eval() const {
  /* declare soon */
  return {};
}

un_operator::un_operator(UnOp type, pointer_type child)
    : type_  {type},
      child_ {child} {}

int un_operator::eval() const {
  /* declare soon */
  return {};
}

ctrl_statement::ctrl_statement(CtrlStatement type, expression* cond,
                              statement_block* body)
   : type_ {type},
     condition_ {cond},
     body_ {body} {}

variable::variable(const std::string& str): name_ {str} {}
variable::variable(std::string&& str): name_ {std::move(str)} {}

int variable::eval() const {
  /* declare soon */
  return {};
}

number::number(int num): value_ {num} {}

int number::eval() const {
  /* declare soon */
  return value_;
}


} // <--- namespace ast

} // <--- namespace frontend
