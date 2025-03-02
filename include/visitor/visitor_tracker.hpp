#pragma once

#include "semantic_context.hpp"
#include "statement.hpp"
#include "values.hpp"
#include "visitor.hpp"

namespace paracl {

class VisitorTracker : public VisitorBase {
protected:
  using ValueTypePtr = PCLValue *;
  using TypeID = SymTable::TypeID;

  VisitorTracker() = default;

  ValueTypePtr get_value() const noexcept { return CurrValue; }

  void set_value(ValueTypePtr Val) noexcept { CurrValue = Val; }

  template <DerivedFromPCLValue ValueType = PCLValue>
  ValueType *getValueAfterAccept(ast::statement *Stm) {
    Stm->accept(this);
    assert(CurrValue);
    return static_cast<ValueType *>(CurrValue);
  }

  ValueTypePtr CurrValue = nullptr;
};

} // namespace paracl
