#pragma once

#include "semantic_context.hpp"
#include "statement.hpp"
#include "values.hpp"
#include "visitor.hpp"

namespace paracl {

class VisitorTracker : public VisitorBase {
public:
  using Type = PCLType;
  using TypeID = SymTable<Type>::TypeID;

  using ValueType = PCLValue;
  using ValuePtr = ValueType *;

protected:
  VisitorTracker() = default;

  ValuePtr getValue() const noexcept { return CurrValue; }

  virtual void setValue(ValuePtr Val) { CurrValue = Val; }

  template <DerivedFromPCLValue ValueT = PCLValue>
  ValueT *getValueAfterAccept(ast::statement *Stm) {
    Stm->accept(this);
    return static_cast<ValueT *>(CurrValue);
  }

  ValuePtr CurrValue = nullptr;
};

} // namespace paracl
