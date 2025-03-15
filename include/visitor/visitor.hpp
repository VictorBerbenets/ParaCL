#pragma once

namespace paracl {

namespace ast {

class ArrayAccessAssignment;
class statement;
class root_statement_block;
class statement_block;
class logic_expression;
class calc_expression;
class assignment;
class definition;
class un_operator;
class number;
class variable;
class if_operator;
class while_operator;
class print_function;
class read_expression;
class ArrayAccess;
class PresetArray;
class UniformArray;
class ArrayHolder;

} // namespace ast

class VisitorBase {
public:
  virtual ~VisitorBase() {};

  virtual void visit(ast::root_statement_block *stm) = 0;
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
  virtual void visit(ast::ArrayHolder *InitListArr) = 0;
  virtual void visit(ast::PresetArray *InitListArr) = 0;
  virtual void visit(ast::ArrayAccess *ArrAccess) = 0;
  virtual void visit(ast::UniformArray *Arr) = 0;
  virtual void visit(ast::ArrayAccessAssignment *Arr) = 0;
};

} // namespace paracl
