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
      curr_value_ = accept(stm->left()) + accept(stm->right());
      break;
    case ast::CalcOp::SUB :
      curr_value_ = accept(stm->left()) - accept(stm->right());
      break;
    case ast::CalcOp::MUL :
      curr_value_ = accept(stm->left()) * accept(stm->right());
      break;
    case ast::CalcOp::PERCENT :
      curr_value_ = accept(stm->left()) % accept(stm->right());
      break;
    case ast::CalcOp::DIV :
      if (auto check = accept(stm->right()); check) {
        curr_value_ = accept(stm->left()) / check;
      } else {
        stm->print_error("division by zero");
        throw std::runtime_error{"trying to divide by 0"};
      }
      break;
    default: throw std::logic_error{"unrecognized logic type"};
  }
}

void interpreter::visit(ast::un_operator *stm) {
  switch(stm->type()) {
    case ast::UnOp::PLUS :
      curr_value_ = +(accept(stm->arg()));
      break;
    case ast::UnOp::MINUS :
      curr_value_ = -(accept(stm->arg()));
      break;
    default: throw std::logic_error{"unrecognized logic type"};
  }
}

void interpreter::visit(ast::logic_expression *stm) {
  switch(stm->type()) {
    case ast::LogicOp::LESS :
      curr_value_ = accept(stm->left()) <  accept(stm->right());
      break;
    case ast::LogicOp::LESS_EQ :
      curr_value_ = accept(stm->left()) <= accept(stm->right());
      break;
    case ast::LogicOp::LOGIC_AND :
      curr_value_ = accept(stm->left()) && accept(stm->right());
      break;
    case ast::LogicOp::LOGIC_OR :
      curr_value_ = accept(stm->left()) || accept(stm->right());
      break;
    case ast::LogicOp::GREATER:
      curr_value_ = accept(stm->left()) > accept(stm->right());
      break;
    case ast::LogicOp::GREATER_EQ :
      curr_value_ = accept(stm->left()) >= accept(stm->right());
      break;
    case ast::LogicOp::EQ :
      curr_value_ = accept(stm->left()) == accept(stm->right());
      break;
    case ast::LogicOp::NEQ :
      curr_value_ = accept(stm->left()) != accept(stm->right());
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
  } else {
    stm->print_error(stm->name() + " was not declared in this scope");
    throw std::runtime_error("not known variable");
  }
}

void interpreter::visit(ast::if_operator *stm) {
  if(accept(stm->condition())) {
    stm->accept_body(this);
  }
}

void interpreter::visit(ast::while_operator *stm) {
  while(accept(stm->condition())) {
    stm->accept_body(this);
  }
}

void interpreter::visit(ast::read_expression *stm) {
  input_stream_ >> curr_value_;
}

void interpreter::visit(ast::print_function *stm) {
  accept(stm->get());
  output_stream_ << curr_value_ << std::endl;
}

void interpreter::visit(ast::assignment *stm) {
  curr_value_ = accept(stm->ident_exp());
  stm->redefine(curr_value_);
}

} // <--- namespace frontend
