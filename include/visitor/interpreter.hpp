#pragma once

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/SmallVector.h>

#include <istream>

#include "identifiers.hpp"
#include "semantic_context.hpp"
#include "statement.hpp"
#include "visitor.hpp"

namespace paracl {

class PCLValueWrapper : public ValueWrapper,
                        public ValueWrapperInterface<PCLValue> {
public:
  using WrapperInterfaceTy::WrapperInterfaceTy;
};

class InterpreterBase : public VisitorBase {
public:
  using WrapperTy = PCLValueWrapper;
  using TypeID = PCLType::TypeID;
  using ResultTy = WrapperTy &;

  void run_program(ast::root_statement_block *StmBlock) { visit(StmBlock); }

protected:
  InterpreterBase() = default;

  ResultTy performLogicalOperation(ast::LogicOp Op, IntegerVal *Lhs,
                                   IntegerVal *Rhs, IntegerTy *Type);
  ResultTy performUnaryOperation(ast::UnOp Op, IntegerVal *Val,
                                 IntegerTy *Type);
  ResultTy performArithmeticOperation(ast::CalcOp Op, IntegerVal *Lhs,
                                      IntegerVal *Rhs, IntegerTy *Type,
                                      yy::location Loc);

  ResultTy acceptStatementBlock(ast::statement_block *StmBlock);

  ResultTy createWrapperRef(PCLValue *Val = nullptr) {
    return VisitorBase::createWrapperRef<WrapperTy>(Val);
  }

  SymTable<PCLType> SymTbl;
  ValueManager<PCLValue> ValManager;
};

class Interpreter : public InterpreterBase {
public:
  Interpreter(std::istream &input, std::ostream &output)
      : input_stream_(input), output_stream_(output) {}

  ResultTy visit(ast::ArrayHolder *ArrStore) override;
  ResultTy visit(ast::ArrayAccessAssignment *Arr) override;
  ResultTy visit(ast::PresetArray *PresArr) override;
  ResultTy visit(ast::ArrayAccess *ArrAccess) override;
  ResultTy visit(ast::UniformArray *UnifArr) override;
  ResultTy visit(ast::calc_expression *CalcExpr) override;
  ResultTy visit(ast::un_operator *UnOp) override;
  ResultTy visit(ast::logic_expression *LogExpr) override;
  ResultTy visit(ast::number *Num) override;
  ResultTy visit(ast::variable *Var) override;
  ResultTy visit(ast::assignment *Assign) override;
  ResultTy visit(ast::read_expression *ReadExpr) override;
  ResultTy visit(ast::statement_block *StmBlock) override;
  ResultTy visit(ast::root_statement_block *StmBlock) override;
  ResultTy visit(ast::if_operator *If) override;
  ResultTy visit(ast::while_operator *While) override;
  ResultTy visit(ast::print_function *Print) override;

private:
  ResultTy acceptASTNode(ast::statement *Stm) override {
    return static_cast<ResultTy>(Stm->accept(this));
  }

  void freeResources(ast::statement_block *StmBlock);
  void addResourceForFree(PCLValue *ValToFree,
                          ast::statement_block *ScopeToFree);

  std::istream &input_stream_;
  std::ostream &output_stream_;
  llvm::DenseMap<ast::statement_block *, llvm::SmallVector<PCLValue *>>
      ResourceHandleMap;
};

} // namespace paracl
