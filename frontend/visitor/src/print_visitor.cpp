#include "print_visitor.hpp"
#include "ast_includes.hpp"

namespace frontend {

print_visitor::print_visitor(const std::string &file_name)
    : o_file_ {file_name} {}

void print_visitor::visit(ast::statement_block *stm) {
  print_tabs();
  o_file_ << "Statement_block" << std::endl;
  ++tabs_number_;
  for (auto&& statement : *stm) {
    statement->accept(this);
  }
  --tabs_number_;
}


void print_visitor::visit(ast::calc_expression *stm) {
  print_tabs();
  o_file_ << "Bin operator" << std::endl;
  ++tabs_number_;
  stm->accept_left(this);
  stm->accept_right(this);
  --tabs_number_;

}

void print_visitor::visit(ast::un_operator *stm) {
  print_tabs();
  o_file_ << "Un operator" << std::endl;
  ++tabs_number_;
  stm->accept_arg(this);
  --tabs_number_;
}

void print_visitor::visit(ast::logic_expression *stm) {
  print_tabs();
  o_file_ << "Logic expression" << std::endl;
  ++tabs_number_;
  stm->accept_left(this);
  stm->accept_right(this);
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

void print_visitor::visit(ast::if_operator *stm) {
  print_tabs();
  o_file_ << "if_operator" << std::endl;
  ++tabs_number_;
  stm->accept_condition(this);
  stm->accept_body(this);
  --tabs_number_;
}

void print_visitor::visit(ast::while_operator *stm) {
  print_tabs();
  o_file_ << "while operator" << std::endl;
  ++tabs_number_;
  stm->accept_condition(this);
  stm->accept_body(this);
  --tabs_number_;
}

void print_visitor::visit(ast::read_expression* /* */) {
  print_tabs();
  o_file_ << "scan_function" << std::endl;

}

void print_visitor::visit(ast::print_function* /* */) {
  print_tabs();
  o_file_ << "print_function" << std::endl;

}

void print_visitor::visit(ast::assignment *stm) {
  print_tabs();
  ++tabs_number_;
  o_file_ << "var definition: " << stm->name() << std::endl;
  stm->accept_exp(this);
  --tabs_number_;
}


void print_visitor::print_tabs() {
  for (int i = 0; i < tabs_number_; ++i) {
    o_file_ << '\t';
  }
}

} // <--- namespace frontend
