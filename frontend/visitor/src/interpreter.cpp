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
  switch(stm->type()) {
    case ast::CalcOp::ADD :
      curr_value_ = evaluate(stm->left()) + evaluate(stm->right());
      break;
    case ast::CalcOp::SUB :
      curr_value_ = evaluate(stm->left()) - evaluate(stm->right());
      break;
    case ast::CalcOp::MUL :
      curr_value_ = evaluate(stm->left()) * evaluate(stm->right());
      break;
    case ast::CalcOp::PERCENT :
      curr_value_ = evaluate(stm->left()) % evaluate(stm->right());
      break;
    case ast::CalcOp::DIV :
      if (auto check = evaluate(stm->right()); check) {
        curr_value_ = evaluate(stm->left()) / check;
      } else {
        throw std::runtime_error{"trying to divide by 0"};
      }
      break;
    default: throw std::logic_error{"unrecognized logic type"};
  }
}

void interpreter::visit(ast::un_operator *stm) {
  switch(stm->type()) {
    case ast::UnOp::PLUS :
      curr_value_ = +(evaluate(stm->arg()));
      break;
    case ast::UnOp::MINUS :
      curr_value_ = -(evaluate(stm->arg()));
      break;
    case ast::UnOp::NEGATE :
      curr_value_ = !(evaluate(stm->arg()));
      break;
    default: throw std::logic_error{"unrecognized logic type"};
  }
}

void interpreter::visit(ast::logic_expression *stm) {
  switch(stm->type()) {
    case ast::LogicOp::LESS :
      curr_value_ = evaluate(stm->left()) <  evaluate(stm->right());
      break;
    case ast::LogicOp::LESS_EQ :
      curr_value_ = evaluate(stm->left()) <= evaluate(stm->right());
      break;
    case ast::LogicOp::LOGIC_AND :
      curr_value_ = evaluate(stm->left()) && evaluate(stm->right());
      break;
    case ast::LogicOp::LOGIC_OR :
      curr_value_ = evaluate(stm->left()) || evaluate(stm->right());
      break;
    case ast::LogicOp::GREATER:
      curr_value_ = evaluate(stm->left()) > evaluate(stm->right());
      break;
    case ast::LogicOp::GREATER_EQ :
      curr_value_ = evaluate(stm->left()) >= evaluate(stm->right());
      break;
    case ast::LogicOp::EQ :
      curr_value_ = evaluate(stm->left()) == evaluate(stm->right());
      break;
    case ast::LogicOp::NEQ :
      curr_value_ = evaluate(stm->left()) != evaluate(stm->right());
      break;
    default: throw std::logic_error{"unrecognized logic type"};
  }
}

void interpreter::visit(ast::number *stm) {
  curr_value_ = stm->get_value();
}

void interpreter::visit(ast::variable *stm) {
  auto curr_scope = stm->scope();
  if (auto right_scope = curr_scope->find(stm->name()); right_scope) {
    curr_value_ = right_scope->value(stm->name());
  }
}

void interpreter::visit(ast::if_operator *stm) {
  if(evaluate(stm->condition())) {
    stm->body()->accept(this);
  } else if (stm->else_block()) {
    stm->else_block()->accept(this);
  }
}

void interpreter::visit(ast::while_operator *stm) {
  while(evaluate(stm->condition())) {
    stm->body()->accept(this);
  }
}

void interpreter::visit(ast::read_expression* /* unused */) {
  input_stream_ >> curr_value_;
}

void interpreter::visit(ast::print_function *stm) {
  evaluate(stm->get());
  output_stream_ << curr_value_ << std::endl;
}

void interpreter::visit(ast::assignment *stm) {
  curr_value_ = evaluate(stm->ident_exp());
  stm->redefine(curr_value_);
}

} // <--- namespace frontend
