#include <iostream>
#include <stdexcept>
#include <optional>

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
    case ast::CalcOp::DIV :
      curr_value_ = accept(stm->left()) / accept(stm->right());
      break;
    case ast::CalcOp::MUL :
      curr_value_ = accept(stm->left()) * accept(stm->right());
      break;
    case ast::CalcOp::PERCENT :
      curr_value_ = accept(stm->left()) % accept(stm->right());
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
  std::optional<int> var_value;
  for (auto curr_scope = stm->scope(); curr_scope; curr_scope = curr_scope->scope()) {
    if (curr_scope->has(stm->name())) {
      var_value = curr_scope->value(stm->name());
      break;
    }
  }
  if (var_value == std::nullopt) {
    throw std::logic_error{stm->name() + " was not declared"};
  }
  curr_value_ = var_value.value();

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
  auto curr_block = stm->scope();
  auto var_name   = stm->var_name();
  for ( ; curr_block ; curr_block = curr_block->scope()) {
    if (curr_block->has(var_name)) {
      break;
    }
  }
  int tmp;
  std::cin >> tmp;
  curr_block->set(var_name, tmp);
}

void interpreter::visit(ast::print_function *stm) {
  auto print_val = stm->get();
  if (std::holds_alternative<std::string>(print_val)) {
    const auto &str_val = std::get<std::string>(print_val);
    for (auto curr_block = stm->scope(); curr_block ; curr_block = curr_block->scope()) {
      if (curr_block->has(str_val)) {
        std::cout << curr_block->value(str_val) << std::endl;
        return ;
      }
    }
    throw std::logic_error{str_val + " was not declared"};
  } else {
    std::cout << std::get<int>(print_val) << std::endl;
  }
}

void interpreter::visit(ast::assignment *stm) {
  curr_value_ = accept(stm->ident_exp());
  stm->redefine(curr_value_);
}

} // <--- namespace frontend
