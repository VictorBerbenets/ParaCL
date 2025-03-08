#pragma once

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/SmallVector.h>

#include <istream>

#include "statement.hpp"
#include "visitor.hpp"
#include "visitor_tracker.hpp"

namespace paracl {

class InterpreterBase : public VisitorTracker {
public:
  void run_program(ast::root_statement_block *StmBlock) { visit(StmBlock); }

protected:
  InterpreterBase() = default;
  ValueTypePtr performLogicalOperation(ast::LogicOp Op, IntegerVal *Lhs,
                                       IntegerVal *Rhs, IntegerTy *Type);
  ValueTypePtr performUnaryOperation(ast::UnOp Op, IntegerVal *Val,
                                     IntegerTy *Type);
  ValueTypePtr performArithmeticOperation(ast::CalcOp Op, IntegerVal *Lhs,
                                          IntegerVal *Rhs, IntegerTy *Type,
                                          yy::location Loc);

  SymTable SymTbl;
  ValueManager ValManager;
};

class Interpreter : public InterpreterBase {
public:
  Interpreter(std::istream &input, std::ostream &output)
      : input_stream_{input}, output_stream_{output} {}

  void visit(ast::ArrayAccessAssignment *Arr) override;

  void visit(ast::PresetArray *InitListArr) override;
  void visit(ast::ArrayAccess *ArrAccess) override;
  void visit(ast::UniformArray *Arr) override;

  void visit(ast::calc_expression *CalcExpr) override;
  void visit(ast::un_operator *UnOp) override;
  void visit(ast::logic_expression *LogExpr) override;
  void visit(ast::number *Num) override;
  void visit(ast::variable *Var) override;
  void visit(ast::assignment *Assign) override;
  void visit(ast::read_expression *ReadExpr) override;
  void visit(ast::statement_block *StmBlock) override;
  void visit(ast::if_operator *If) override;
  void visit(ast::while_operator *While) override;
  void visit(ast::print_function *Print) override;

private:
  void freeResources(ast::statement_block *StmBlock);
  void addResourceForFree(PCLValue *ValToFree,
                          ast::statement_block *ScopeToFree);

  std::istream &input_stream_;
  std::ostream &output_stream_;
  llvm::DenseMap<ast::statement_block *, llvm::SmallVector<PCLValue *>>
      ResourceHandleMap;
};

} // namespace paracl
