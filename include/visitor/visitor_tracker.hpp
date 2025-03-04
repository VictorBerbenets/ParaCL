#pragma once

#include "identifiers.hpp"
#include "location.hh"
#include "semantic_context.hpp"
#include "statement.hpp"
#include "values.hpp"
#include "visitor.hpp"

namespace paracl {

class VisitorTracker : public VisitorBase {
public:
  void run_program(ast::root_statement_block *StmBlock) { visit(StmBlock); }

protected:
  using ValueTypePtr = PCLValue *;
  using TypeID = SymTable::TypeID;

  VisitorTracker() = default;
  ValueTypePtr performLogicalOperation(ast::LogicOp Op, IntegerVal *Lhs,
                                       IntegerVal *Rhs, IntegerTy *Type);
  ValueTypePtr performUnaryOperation(ast::UnOp Op, IntegerVal *Val,
                                     IntegerTy *Type);
  ValueTypePtr performArithmeticOperation(ast::CalcOp Op, IntegerVal *Lhs,
                                          IntegerVal *Rhs, IntegerTy *Type,
                                          yy::location Loc);

  ValueTypePtr get_value() const noexcept { return CurrValue; }

  virtual void set_value(ValueTypePtr Val) { CurrValue = Val; }

  template <DerivedFromPCLValue ValueType = PCLValue>
  ValueType *getValueAfterAccept(ast::statement *Stm) {
    Stm->accept(this);
    return static_cast<ValueType *>(CurrValue);
  }

  SymTable SymTbl;
  ValueManager ValManager;
  ValueTypePtr CurrValue = nullptr;
};

} // namespace paracl
