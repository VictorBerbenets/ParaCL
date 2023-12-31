#pragma once

#include <string>
#include <fstream>

#include "visitor.hpp"

namespace frontend {

class print_visitor: public base_visitor {
  void print_tabs();
 public:
  explicit print_visitor(const std::string &file_name);

  void visit(ast::statement_block *stm)  override;
  void visit(ast::calc_expression *stm)  override;
  void visit(ast::un_operator *stm)      override;
  void visit(ast::logic_expression *stm) override;
  void visit(ast::number *stm)           override;
  void visit(ast::variable *stm)         override;
  void visit(ast::assignment *stm)       override;
  void visit(ast::if_operator *stm)      override;
  void visit(ast::while_operator *stm)   override;
  void visit(ast::read_expression *stm)  override;
  void visit(ast::print_function *stm)   override;
 private:
  std::ofstream o_file_;
  int tabs_number_ = 0;
};

}
