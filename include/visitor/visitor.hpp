#pragma once

#include "forward_decls.hpp"

namespace paracl {

class base_visitor {
public:
  virtual ~base_visitor() {};

  virtual void visit(ast::statement_block *StmBlock) = 0;
  virtual void visit(ast::calc_expression *CalcExpr) = 0;
  virtual void visit(ast::logic_expression *LogExpr) = 0;
  virtual void visit(ast::un_operator *UnOper) = 0;
  virtual void visit(ast::number *Num) = 0;
  virtual void visit(ast::variable *Var) = 0;
  virtual void visit(ast::assignment *Assign) = 0;
  virtual void visit(ast::if_operator *If) = 0;
  virtual void visit(ast::while_operator *While) = 0;
  virtual void visit(ast::read_expression *ReadExpr) = 0;
  virtual void visit(ast::print_function *Print) = 0;
  virtual void visit(ast::InitListArray *InitListArr) = 0;
  virtual void visit(ast::ArrayAccess *ArrAccess) = 0;
  virtual void visit(ast::UndefVar *UndVar) = 0;
  virtual void visit(ast::Array *Arr) = 0;
};

} // namespace paracl
