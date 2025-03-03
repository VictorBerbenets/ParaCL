#pragma once

#include <llvm/ADT/DenseMap.h>
#include "llvm/Support/raw_ostream.h"

#include "types.hpp"
#include "values.hpp"

namespace paracl {

class SymTable final {
public:
  using TypeID = PCLType::TypeID;

  struct SymbInfo final {
  public:
    SymbInfo(PCLType *Ty) : Ty(Ty) {}
    SymbInfo(SymTable &SymTbl, TypeID ID) : Ty(SymTbl.getType(ID)) {}

    PCLType *getType() { return Ty; }

  private:
    PCLType *Ty;
  };

  PCLType *getType(TypeID ID) {
    if (Types.contains(ID))
      return Types[ID].get();

    switch (ID) {
    case TypeID::Int32:
      Types.try_emplace(ID, std::make_unique<IntegerTy>(ID));
      break;
    case TypeID::UniformArray:
      Types.try_emplace(ID, std::make_unique<ArrayTy>(ID));
      break;
    case TypeID::PresetArray:
      Types.try_emplace(ID, std::make_unique<ArrayTy>(ID));
      break;
    default:
      Types.try_emplace(TypeID::Unknown,
                        std::make_unique<PCLType>(TypeID::Unknown));
    }
    return Types[ID].get();
  }

  template <typename... ArgsTy>
  bool tryDefine(const SymbNameType &Name, ast::statement_block *CurrScope,
                 ArgsTy &&...Args) {
    if (isDefined({Name, CurrScope}))
      return false;

    llvm::outs() << Name << " is defined\n";
    auto [_, IsEmplaced] = NamesInfo.try_emplace(
        {Name, CurrScope}, SymbInfo(std::forward<ArgsTy>(Args)...));
    assert(IsEmplaced && "can't emplace the symbol");
    return true;
  }

  ast::statement_block *getDeclScopeFor(const SymbNameType &Name,
                                        ast::statement_block *CurrScope);

  PCLType *getTypeFor(const SymbNameType &Name,
                      ast::statement_block *CurrScope);

  bool isDefined(SymTabKey TabKey);

  friend class driver;

private:
  llvm::DenseMap<SymTabKey, SymbInfo> NamesInfo;
  llvm::DenseMap<TypeID, std::unique_ptr<PCLType>> Types;
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
