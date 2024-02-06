#include <iostream>
#include <stdexcept>

#include "ast_includes.hpp"
#include "interpreter.hpp"
#include "identifiers.hpp"

namespace frontend {

void interpreter::visit(ast::statement_block *stm) {
  for (auto&& statement : *stm) {
    statement->accept(this);
  }
}

void interpreter::visit(ast::calc_expression *stm) {
  stm->left()->accept(this);
  auto lhs = get_value();
  stm->right()->accept(this);
  auto rhs = get_value();

  switch(stm->type()) {
    case ast::CalcOp::ADD :
      set_value(lhs + rhs);
      break;
    case ast::CalcOp::SUB :
      set_value(lhs - rhs);
      break;
    case ast::CalcOp::MUL :
      set_value(lhs * rhs);
      break;
    case ast::CalcOp::PERCENT :
      set_value(lhs % rhs);
      break;
    case ast::CalcOp::DIV :
      if (auto check = rhs; check) {
        set_value(lhs / check);
      } else {
        throw std::runtime_error{"trying to divide by 0"};
      }
      break;
    default: throw std::logic_error{"unrecognized logic type"};
  }
}

void interpreter::visit(ast::un_operator *stm) {
  stm->arg()->accept(this);

  switch(stm->type()) {
    case ast::UnOp::PLUS :
      set_value(+get_value());
      break;
    case ast::UnOp::MINUS :
      set_value(-get_value());
      break;
    case ast::UnOp::NEGATE :
      set_value(!get_value());
      break;
    default: throw std::logic_error{"unrecognized logic type"};
  }
}

void interpreter::visit(ast::logic_expression *stm) {
  stm->left()->accept(this);
  auto lhs = get_value();
  stm->right()->accept(this);
  auto rhs = get_value();

  switch(stm->type()) {
    case ast::LogicOp::LESS :
      set_value(lhs <  rhs);
      break;
    case ast::LogicOp::LESS_EQ :
      set_value(lhs <= rhs);
      break;
    case ast::LogicOp::LOGIC_AND :
      set_value(lhs && rhs);
      break;
    case ast::LogicOp::LOGIC_OR :
      set_value(lhs || rhs);
      break;
    case ast::LogicOp::GREATER:
      set_value(lhs > rhs);
      break;
    case ast::LogicOp::GREATER_EQ :
      set_value(lhs >= rhs);
      break;
    case ast::LogicOp::EQ :
      set_value(lhs == rhs);
      break;
    case ast::LogicOp::NEQ :
      set_value(lhs != rhs);
      break;
    default: throw std::logic_error{"unrecognized logic type"};
  }
}

void interpreter::visit(ast::number *stm) {
  set_value(stm->get_value());
}

void interpreter::visit(ast::variable *stm) {
  auto curr_scope  = stm->scope();
  auto right_scope = curr_scope->find(stm->name());
  set_value(right_scope->value(stm->name()));
}

void interpreter::visit(ast::if_operator *stm) {
  stm->condition()->accept(this);

  if(get_value()) {
    stm->body()->accept(this);
  } else if (stm->else_block()) {
    stm->else_block()->accept(this);
  }
}

void interpreter::visit(ast::while_operator *stm) {
  stm->condition()->accept(this);

  while(get_value()) {
    stm->body()->accept(this);
    stm->condition()->accept(this);
  }
}

void interpreter::visit(ast::read_expression* /* unused */) {
  int tmp {0};
  input_stream_ >> tmp;
  set_value(tmp);
}

void interpreter::visit(ast::print_function *stm) {
  stm->get()->accept(this);
  output_stream_ << get_value() << std::endl;
}

void interpreter::visit(ast::assignment *stm) {
//  set_value(evaluate(stm->ident_exp());
  stm->ident_exp()->accept(this);
  stm->redefine(get_value());
}

} // <--- namespace frontend
