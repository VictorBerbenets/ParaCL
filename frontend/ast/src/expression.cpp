#include <stdexcept>

#include "expression.hpp"
#include "visitor.hpp"

namespace frontend {

namespace ast {

logic_expression::logic_expression(LogicOp type, expression *left, expression *right)
    : type_ {type},
      left_ {left},
      right_ {right} {}

int logic_expression::eval() const {
  switch(type_) {
    case LogicOp::EQ :         return left_->eval() == right_->eval();
    case LogicOp::NEQ :        return left_->eval() != right_->eval();
    case LogicOp::LESS :       return left_->eval() <  right_->eval();
    case LogicOp::LESS_EQ :    return left_->eval() <= right_->eval();
    case LogicOp::GREATER :    return left_->eval() >  right_->eval();
    case LogicOp::GREATER_EQ : return left_->eval() >= right_->eval();
    case LogicOp::LOGIC_AND :
      if (!left_->eval()) {
        return false;
      }
      return right_->eval();
    case LogicOp::LOGIC_OR :
      if (left_->eval()) {
        return true;
      }
      return right_->eval();
    default : throw std::logic_error{"invalid logic operator"};
  }
}

void logic_expression::accept(base_visitor* b_visitor) {
  b_visitor->visit(this);
}

expression *logic_expression::left() const noexcept {
  return left_;
}

expression *logic_expression::right() const noexcept {
  return right_;
}

bin_operator::bin_operator(BinOp type, pointer_type left, pointer_type right)
    : type_  {type},
      left_  {left},
      right_ {right} {}

int bin_operator::eval() const {
  switch(type_) {
    case BinOp::ADD : return left_->eval() + right_->eval();
    case BinOp::SUB : return left_->eval() - right_->eval();
    case BinOp::MUL : return left_->eval() * right_->eval();
    case BinOp::DIV : return left_->eval() / right_->eval();
    default : throw std::logic_error{"invalid logic operator"};
  }
}

void bin_operator::accept(base_visitor* b_visitor) {
  b_visitor->visit(this);
}

expression *bin_operator::left() const noexcept {
  return left_;
}

expression *bin_operator::right() const noexcept {
  return right_;
}

un_operator::un_operator(UnOp type, pointer_type child)
    : type_  {type},
      child_ {child} {}

int un_operator::eval() const {
  /* declare soon */
  return {};
}

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

int variable::eval() const {

  return 0;
}

void variable::accept(base_visitor *b_visitor) {
  b_visitor->visit(this);
}

statement_block *variable::scope() noexcept {
  return parent_;
}

number::number(int num): value_ {num} {}

int number::eval() const {
  /* declare soon */
  return value_;
}

void number::accept(base_visitor *b_visitor) {
  b_visitor->visit(this);
}

const number::value_type &number::get_value() const noexcept {
  return value_;
}

} // <--- namespace ast

} // <--- namespace frontend
