#pragma once

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

  ValueTypePtr get_value() const noexcept { return CurrValue; }

  void set_value(ValueTypePtr Val) noexcept { CurrValue = Val; }

  template <DerivedFromPCLValue ValueType = PCLValue>
  ValueType *getValueAfterAccept(ast::statement *Stm) {
    Stm->accept(this);
    return static_cast<ValueType *>(CurrValue);
  } 

  ValueTypePtr CurrValue = nullptr;
};

} // namespace paracl
