#pragma once

#include "llvm/Support/raw_ostream.h"
#include <llvm/ADT/DenseMap.h>

#include "types.hpp"
#include "values.hpp"

namespace paracl {

class SymTable final {
public:
  using TypeID = PCLType::TypeID;

  struct SymbInfo final {
  public:
    SymbInfo(PCLType *Ty) : Ty(Ty) {}
    SymbInfo(SymTable &SymTbl, TypeID ID) : Ty(SymTbl.createType(ID)) {}

    PCLType *getType() { return Ty; }

  private:
    PCLType *Ty;
  };

  PCLType *createType(TypeID ID) {
    switch (ID) {
    case TypeID::Int32:
      Types.emplace_back(std::make_unique<IntegerTy>(ID));
      break;
    case TypeID::UniformArray:
      Types.emplace_back(std::make_unique<ArrayTy>(ID));
      break;
    case TypeID::PresetArray:
      Types.emplace_back(std::make_unique<ArrayTy>(ID));
      break;
    case TypeID::Unknown:
      Types.emplace_back(std::make_unique<PCLType>(TypeID::Unknown));
    default:
      llvm_unreachable("Unknown type ID");
    }
    assert(!Types.empty());
    return Types.back().get();
  }

  template <typename... ArgsTy>
  bool tryDefine(const SymbNameType &Name, ast::statement_block *CurrScope,
                 ArgsTy &&...Args) {
    auto [_, IsEmplaced] = NamesInfo.try_emplace(
        {Name, CurrScope}, SymbInfo(std::forward<ArgsTy>(Args)...));
    return IsEmplaced;
  }

  ast::statement_block *getDeclScopeFor(const SymbNameType &Name,
                                        ast::statement_block *CurrScope);

  PCLType *getTypeFor(const SymbNameType &Name,
                      ast::statement_block *CurrScope);

  bool isDefined(SymTabKey TabKey);

  friend class driver;

private:
  llvm::DenseMap<SymTabKey, SymbInfo> NamesInfo;
  llvm::SmallVector<std::unique_ptr<PCLType>> Types;
};

class ValueManager final {
public:
  template <DerivedFromPCLValue ValueTy, typename... ArgTys>
  PCLValue *createValueFor(SymTabKey &&SymKey, ArgTys &&...Args) {
    auto ValuePtr = std::make_unique<ValueTy>(std::forward<ArgTys>(Args)...);
    Values.push_back(std::move(ValuePtr));
    auto [InsIt, _] = NameToValue.insert_or_assign(
        std::forward<SymTabKey>(SymKey), Values.back().get());
    return InsIt->second;
  }

  bool linkValueWithName(SymTabKey &&SymKey, PCLValue *Val) {
    auto [_, IsInsert] =
        NameToValue.insert_or_assign(std::forward<SymTabKey>(SymKey), Val);
    return IsInsert;
  }

  template <DerivedFromPCLValue ValueTy, typename... ArgTys>
  PCLValue *createValue(ArgTys &&...Args) {
    Values.emplace_back(
        std::make_unique<ValueTy>(std::forward<ArgTys>(Args)...));
    return Values.back().get();
  }

  template <DerivedFromPCLValue ValueType = PCLValue>
  ValueType *getValueFor(SymTabKey SymKey) {
    if (!NameToValue.contains(SymKey))
      return nullptr;
    return static_cast<ValueType *>(NameToValue[SymKey]);
  }

  friend class driver;

private:
  llvm::DenseMap<SymTabKey, PCLValue *> NameToValue;
  llvm::SmallVector<std::unique_ptr<PCLValue>> Values;
};

} // namespace paracl
