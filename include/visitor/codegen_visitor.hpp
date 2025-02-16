#pragma once

#include "visitor.hpp"

namespace paracl {

class CodeGenVisitor: public base_visitor {
public: 
  void visit(ast::calc_expression *stm)  override;
  void visit(ast::un_operator *stm)      override;
  void visit(ast::logic_expression *stm) override;
  void visit(ast::number *stm)           override;
  void visit(ast::variable *stm)         override;
  void visit(ast::assignment *stm)       override;
  void visit(ast::read_expression *stm)  override;
  void visit(ast::statement_block *stm)  override;
  void visit(ast::if_operator *stm)      override;
  void visit(ast::while_operator *stm)   override;
  void visit(ast::print_function *stm)   override;
  
  virtual void visit(ast::definition *stm) = 0;

};

} // paracl
