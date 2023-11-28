#include <stdexcept>

#include "expression.hpp"

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
  return 0;
}

number::number(int num): value_ {num} {}

int number::eval() const {
  /* declare soon */
  return value_;
}


} // <--- namespace ast

} // <--- namespace frontend
