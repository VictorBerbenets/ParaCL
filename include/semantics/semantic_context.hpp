#pragma once

#include <llvm/ADT/DenseMap.h>
#include <llvm/Support/raw_ostream.h>

#include "types.hpp"
#include "values.hpp"

namespace paracl {

class SymTable final {
public:
  using TypeID = PCLType::TypeID;

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

  bool tryDefine(const SymTabKey &Key, TypeID ID) {
    auto [_, IsEmplaced] =
        NamesInfo.try_emplace(Key, createType(ID));
    return IsEmplaced;
  }
  
  bool tryDefine(const SymTabKey &Key, PCLType *Ty) {
    auto [_, IsEmplaced] =
        NamesInfo.try_emplace(Key, Ty);
    return IsEmplaced;
  }

  SymTabKey getDeclKeyFor(const SymbNameType &Name,
                          ast::statement_block *CurrScope);
  SymTabKey getDeclKeyFor(const SymTabKey &Key);

  ast::statement_block *getDeclScopeFor(const SymbNameType &Name,
                                        ast::statement_block *CurrScope);
  ast::statement_block *getDeclScopeFor(const SymTabKey &Key);

  PCLType *getTypeFor(const SymbNameType &Name,
                      ast::statement_block *CurrScope);
  PCLType *getTypeFor(const SymTabKey &Key);

  bool isDefined(SymTabKey TabKey);

  bool containsKeyWithType(PCLType *Ty) const;

  friend class driver;

private:
  llvm::DenseMap<SymTabKey, PCLType*> NamesInfo;
  llvm::SmallVector<std::unique_ptr<PCLType>> Types;
};

class ValueManager final {
public:
  template <DerivedFromPCLValue ValueTy, typename... ArgTys>
  ValueTy *createValueFor(const SymTabKey &SymKey, ArgTys &&...Args) {
    auto ValuePtr = std::make_unique<ValueTy>(std::forward<ArgTys>(Args)...);
    Values.push_back(std::move(ValuePtr));
    auto [InsIt, _] = NameToValue.insert_or_assign(SymKey, Values.back().get());
    return static_cast<ValueTy *>(InsIt->second);
  }

  bool linkValueWithName(const SymTabKey &SymKey, PCLValue *Val) {
    auto [_, IsInsert] = NameToValue.insert_or_assign(SymKey, Val);
    return IsInsert;
  }

  template <DerivedFromPCLValue ValueTy, typename... ArgTys>
  ValueTy *createValue(ArgTys &&...Args) {
    Values.emplace_back(
        std::make_unique<ValueTy>(std::forward<ArgTys>(Args)...));
    return static_cast<ValueTy *>(Values.back().get());
  }

  template <DerivedFromPCLValue ValueType = PCLValue>
  ValueType *getValueFor(SymTabKey SymKey) {
    if (!NameToValue.contains(SymKey))
      return nullptr;
    return static_cast<ValueType *>(NameToValue[SymKey]);
  }

  bool containsValue(PCLValue *Val) const {
    return llvm::find_if(NameToValue, [Val](auto &&MapPair) {
             return MapPair.second == Val;
           }) != NameToValue.end();
  }

  friend class driver;

private:
  llvm::DenseMap<SymTabKey, PCLValue *> NameToValue;
  llvm::SmallVector<std::unique_ptr<PCLValue>> Values;
};

} // namespace paracl
