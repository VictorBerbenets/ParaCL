#pragma once

#include "statement.hpp"
#include "values.hpp"
#include "visitor.hpp"

namespace paracl {

class VisitorTracker : public VisitorBase {
public:
  using ValueTypePtr = PCLValue *;
  using TypeID = SymTable::TypeID;

protected:
  VisitorTracker() = default;

  ValueTypePtr getValue() const noexcept { return CurrValue; }

  virtual void setValue(ValueTypePtr Val) { CurrValue = Val; }

  template <DerivedFromPCLValue ValueType = PCLValue>
  ValueType *getValueAfterAccept(ast::statement *Stm) {
    Stm->accept(this);
    return static_cast<ValueType *>(CurrValue);
  }

  ValueTypePtr CurrValue = nullptr;
};

} // namespace paracl
