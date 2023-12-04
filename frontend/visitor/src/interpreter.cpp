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
      curr_value_ = accept(stm->right());
      if (curr_value_ == 0) { throw std::runtime_error{"trying to divide by 0"}; }
      curr_value_ = accept(stm->left()) / curr_value_;
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
    throw std::logic_error{stm->name() + " was not declared"};
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

void interpreter::visit(ast::scan_function *stm) {
  auto var_name    = stm->var_name();
  auto curr_scope  = stm->scope();
  if (auto final_scope = curr_scope->find(var_name); final_scope) {
    int tmp;
    std::cin >> tmp;
    final_scope->set(var_name, tmp);
  } else {
    throw std::runtime_error{var_name + " was not declared in this scope"};
  }
}

void interpreter::visit(ast::print_function *stm) {
  auto print_val = stm->get();
  if (std::holds_alternative<std::string>(print_val)) {
    const auto &str_val = std::get<std::string>(print_val);
    auto curr_block = stm->scope();
    if (auto right_scope = curr_block->find(str_val); right_scope) {
      std::cout << right_scope->value(str_val) << std::endl;
    } else {
      throw std::logic_error{str_val + " was not declared"};
    }
  } else {
    std::cout << std::get<int>(print_val) << std::endl;
  }
}

void interpreter::visit(ast::assignment *stm) {
  curr_value_ = accept(stm->ident_exp());
  stm->redefine(curr_value_);
}

} // <--- namespace frontend
