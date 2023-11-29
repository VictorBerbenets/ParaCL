#include <iostream>

#include "ast_includes.hpp"
#include "interpreter.hpp"

namespace frontend {

void interpreter::visit(ast::statement *stm) {

}

void interpreter::visit(ast::statement_block *stm) {

}

void interpreter::visit(ast::expression *stm) {

}

void interpreter::visit(ast::bin_operator *stm) {
  curr_value_ = accept(stm->left()) + accept(stm->right());
}

void interpreter::visit(ast::un_operator *stm) {

}

void interpreter::visit(ast::logic_expression *stm) {

}

void interpreter::visit(ast::number *number) {
  std::cout << "NUMBER\n";
  curr_value_ = number->get_value();
}

void interpreter::visit(ast::variable *stm) {

}

void interpreter::visit(ast::ctrl_statement *stm) {

}

void interpreter::visit(ast::scan_function *stm) {

}

void interpreter::visit(ast::print_function *stm) {

}

} // <--- namespace frontend
