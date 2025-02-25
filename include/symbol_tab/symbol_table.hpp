#pragma once

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
  
struct SymTabKey final {
  llvm::StringRef Name;
  ast::statement_block *CurrScope;
};

class SymTable final {
public:

  struct SymbInfo final {
    std::unique_ptr<PCLType> Ty; 
  };

  bool tryDefine(const llvm::StringRef &Name, ast::statement_block *CurrScope, PCLType *) {
    if (isDefined({Name, CurrScope})) 
      return false;
    
    NamesInfo.insert({Name, CurrScope}, nullptr);

  }

  ast::statement_block *getDeclScopeFor(const llvm::StringRef &Name, ast::statement_block *CurrScope);

  PCLType *getTypeFor(const llvm::StringRef &Name, ast::statement_block *CurrScope);
  
  bool isDefined(SymTabKey TabKey) const;
  
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
  void CreateValueFor(const llvm::StringRef &Name, ArgTys &&... Args) {
    auto ValuePtr = std::make_unique<ValueTy>(std::forward<ArgTys>(Args)...);
    NameToValue.insert_or_assign(Name, std::move(ValuePtr));    
  }

  PCLValue *getValueFor(const llvm::StringRef &Name, ast::statement_block *CurrScope) {
    if (NameToValue.contains({Name, CurrScope}))
      return NameToValue[{Name, CurrScope}].get();
    return nullptr;
  }

  friend class driver;

private:
    llvm::DenseMap<SymTabKey, std::unique_ptr<PCLValue>> NameToValue;
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

