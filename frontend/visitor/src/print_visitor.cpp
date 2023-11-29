#include "print_visitor.hpp"
#include "ast_includes.hpp"

namespace frontend {

print_visitor::print_visitor(const std::string &file_name)
    : o_file_ {file_name} {}

void print_visitor::visit(ast::statement *stm) {
  o_file_ << "Statement" << std::endl;
  stm->accept(this);
}

void print_visitor::visit(ast::statement_block *stm) {
  print_tabs();
  o_file_ << "Statement_block" << std::endl;
  ++tabs_number_;
  for (auto&& statement : stm->statements()) {
    statement->accept(this);
  }
  --tabs_number_;
  // stm->
}

void print_visitor::visit(ast::expression *stm) {
  print_tabs();
  o_file_ << "Expression" << std::endl;
  ++tabs_number_;
  stm->accept(this);
  --tabs_number_;
}

void print_visitor::visit(ast::bin_operator *stm) {
  print_tabs();
  o_file_ << "Bin operator" << std::endl;
  ++tabs_number_;
  stm->left()->accept(this);
  stm->right()->accept(this);
  --tabs_number_;

}

void print_visitor::visit(ast::un_operator *stm) {
  print_tabs();
  o_file_ << "Un operator" << std::endl;
  ++tabs_number_;
  stm->arg()->accept(this);
  --tabs_number_;
}

void print_visitor::visit(ast::logic_expression *stm) {
  print_tabs();
  o_file_ << "Logic expression" << std::endl;
  ++tabs_number_;
  stm->left()->accept(this);
  stm->right()->accept(this);
  --tabs_number_;
}

void print_visitor::visit(ast::number *stm) {
  print_tabs();
  o_file_ << "number: " << stm->get_value() << std::endl;

}

void print_visitor::visit(ast::variable *stm) {
  print_tabs();
  o_file_ << "variable: " << stm->name() << std::endl;

}

void print_visitor::visit(ast::ctrl_statement *stm) {
  print_tabs();
  o_file_ << "ctrl_statement" << std::endl;
  ++tabs_number_;
  stm->condition_->accept(this);
  stm->body_->accept(this);
  --tabs_number_;
}

void print_visitor::visit(ast::scan_function *stm) {
  print_tabs();
  o_file_ << "scan_function" << std::endl;

}

void print_visitor::visit(ast::print_function *stm) {
  print_tabs();
  o_file_ << "print_function" << std::endl;

}

void print_visitor::visit(ast::var_definition *stm) {
  print_tabs();
  ++tabs_number_;
  o_file_ << "var definition: " << stm->name_ << std::endl;
  stm->identifier_->accept(this);
  --tabs_number_;
}


void print_visitor::print_tabs() {
  for (int i = 0; i < tabs_number_; ++i) {
    o_file_ << '\t';
  }
}

} // <--- namespace frontend