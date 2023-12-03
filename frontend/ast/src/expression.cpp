#include <stdexcept>

#include "expression.hpp"
#include "visitor.hpp"

namespace frontend {

namespace ast {

un_operator::un_operator(UnOp type, pointer_type child)
    : type_  {type},
      child_ {child} {}

void un_operator::accept(base_visitor* b_visitor) {
  b_visitor->visit(this);
}

expression *un_operator::arg() {
  return child_;
}

variable::variable(statement_block *curr_block, const std::string& str)
    : expression(curr_block),
      name_ {str} {}

variable::variable(statement_block *curr_block, std::string&& str)
    : expression(curr_block),
      name_ {std::move(str)} {}

std::string variable::name() noexcept {
  return name_;
}

void variable::accept(base_visitor *b_visitor) {
  b_visitor->visit(this);
}

number::number(int num): value_ {num} {}

void number::accept(base_visitor *b_visitor) {
  b_visitor->visit(this);
}

const number::value_type &number::get_value() const noexcept {
  return value_;
}

} // <--- namespace ast

} // <--- namespace frontend
