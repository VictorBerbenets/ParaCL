#pragma once
#include <iostream>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/Hashing.h>

#include <memory>
#include <unordered_map>

#include "types.hpp"

namespace paracl {

namespace ast {
class statement_block;
}; // namespace ast

using SymbNameType = llvm::SmallString<16>;

struct SymTabKey final {
  SymbNameType Name;
  ast::statement_block *CurrScope;
};

} // namespace paracl

namespace llvm {

template<>
struct DenseMapInfo<paracl::SymTabKey> {
  static inline paracl::SymTabKey getEmptyKey() {
    return {StringRef(""), nullptr};
  }

  static inline paracl::SymTabKey getTombstoneKey() {
    return {StringRef(""), nullptr};
  }

  static unsigned getHashValue(const paracl::SymTabKey  &TabKey) {
    return llvm::hash_value(TabKey.Name.str()) + llvm::hash_value(TabKey.CurrScope);
  }

  static bool isEqual(const paracl::SymTabKey &LHS, const paracl::SymTabKey &RHS) {
    return LHS.CurrScope == RHS.CurrScope && LHS.Name == RHS.Name;
  }
};

} // namespace llvm

namespace paracl { 

class SymTable final {
  using TypeID = PCLType::TypeID;
public:

  struct SymbInfo final {
  public:
    SymbInfo(TypeID ID): Ty(constructTypeFromId(ID)) {}
    
    PCLType *getType() { return Ty.get(); }
  private:
    std::unique_ptr<PCLType> constructTypeFromId(TypeID ID) {
      switch(ID) {
        case TypeID::Int32:
          return std::make_unique<IntegerTy>();
        case TypeID::Array:
          return std::make_unique<ArrayTy>();
        default:
          return std::make_unique<PCLType>(TypeID::Unknown);
      }
    }

    std::unique_ptr<PCLType> Ty; 
  };
  
  template<typename... ArgsTy>
  bool tryDefine(const SymbNameType &Name, ast::statement_block *CurrScope, ArgsTy &&... Args) {
    if (isDefined({Name, CurrScope})) 
      return false;
 
    //std::cout << "Name = " << std::string(Name) << "; Scope = " << std::hex << CurrScope << '\n';
    auto [_, IsEmplaced] = NamesInfo.try_emplace({Name, CurrScope}, SymbInfo(std::forward<ArgsTy>(Args)...));
    assert(IsEmplaced && "couldn't emplaced the symbol");
   // std::cout << "EMPLACED\n";
    return true;

  }

  ast::statement_block *getDeclScopeFor(const SymbNameType &Name, ast::statement_block *CurrScope);

  PCLType *getTypeFor(const SymbNameType &Name, ast::statement_block *CurrScope);
  
  bool isDefined(SymTabKey TabKey);
  
  friend class driver;

private:
  llvm::DenseMap<SymTabKey, SymbInfo> NamesInfo;
};


class PCLValue {

protected:

  PCLValue() = default;

public:
  virtual ~PCLValue() = default; 
};

class IntegerVal : public PCLValue {
public:

  IntegerVal(int Val): Val (Val) {}
  
  int getValue() const noexcept { return Val; }

private:
  int Val;
};

class ArrayVal: public PCLValue {
public:

  void construct (std::initializer_list<PCLValue *> ValList) {
    Elems = std::vector<PCLValue*>(ValList.begin(), ValList.end());
  }

private:
  std::vector<PCLValue*> Elems;
};

template <typename T>
concept DerivedFromPCLVal = std::derived_from<T, PCLValue>;

class ValueManager final {
public:
  
  template <DerivedFromPCLVal ValueTy, typename... ArgTys>
  void createValueFor(SymTabKey SymKey, ArgTys &&... Args) {
    auto ValuePtr = std::make_unique<ValueTy>(std::forward<ArgTys>(Args)...);
    NameToValue.insert_or_assign(SymKey, std::move(ValuePtr));    
  }

  PCLValue *getValueFor(SymTabKey SymKey) {
    if (!NameToValue.contains(SymKey))
      return nullptr;
    return NameToValue[SymKey].get();
  }

  friend class driver;

private:
    llvm::DenseMap<SymTabKey, std::unique_ptr<PCLValue>> NameToValue;
};

} // namespace paracl
