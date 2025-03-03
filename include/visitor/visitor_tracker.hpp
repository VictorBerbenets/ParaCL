#pragma once

#include "semantic_context.hpp"
#include "statement.hpp"
#include "values.hpp"
#include "visitor.hpp"
#include "identifiers.hpp"

namespace paracl {

class VisitorTracker : public VisitorBase {
public:
  void run_program(ast::root_statement_block *StmBlock) { visit(StmBlock); }

protected:
  using ValueTypePtr = PCLValue *;
  using TypeID = SymTable::TypeID;

  VisitorTracker() = default;
  ValueTypePtr performLogicalOperation(ast::LogicOp Op, IntegerVal *Lhs, IntegerVal *Rhs, IntegerTy *Type);


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
