#pragma once

#include <llvm/ADT/DenseMap.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/raw_ostream.h>

#include "statement.hpp"
#include "types.hpp"
#include "values.hpp"

namespace paracl {

// The base class for tracking all variables. The variable is added to the table
// along with the scope where it is located. This way we know where it was first
// defined. This is also necessary for error handling.
template <typename Type> class SymTableBase {
public:
  using ValueType = Type;
  using PtrType = ValueType *;

  virtual ~SymTableBase() {}

  // Attempts to define a variable. Returns false if it has already been
  // declared, and true otherwise.
  bool tryDefine(const SymTabKey &Key, PtrType Ty) {
    auto [_, IsEmplaced] = NamesInfo.try_emplace(Key, Ty);
    return IsEmplaced;
  }

  //  Returns the scope where the variable with the name Name was defined (first
  //  assigned) within the current scope CurrScope.
  ast::statement_block *getDeclScopeFor(const SymbNameType &Name,
                                        ast::statement_block *CurrScope) {
    for (; CurrScope; CurrScope = CurrScope->scope()) {
      if (NamesInfo.contains({Name, CurrScope}))
        return CurrScope;
    }
    return nullptr;
  }

  ast::statement_block *getDeclScopeFor(const SymTabKey &Key) {
    return getDeclScopeFor(Key.Name, Key.CurrScope);
  }

  // Returns the declaration key of the variable with the name Name that was
  // defined (first assigned) within the current scope CurrScope
  SymTabKey getDeclKeyFor(const SymbNameType &Name,
                          ast::statement_block *CurrScope) {
    auto *DeclScope = getDeclScopeFor(Name, CurrScope);
    return {Name, DeclScope};
  }

  SymTabKey getDeclKeyFor(const SymTabKey &Key) {
    auto *DeclScope = getDeclScopeFor(Key.Name, Key.CurrScope);
    return {Key.Name, DeclScope};
  }

  // Get the type of the variable with the name Name within the current scope
  // CurrScope.
  PCLType *getTypeFor(const SymbNameType &Name,
                      ast::statement_block *CurrScope) {
    auto *Decl = getDeclScopeFor(Name, CurrScope);
    if (!Decl)
      return nullptr;

    return NamesInfo.find({Name, Decl})->second;
  }

  PCLType *getTypeFor(const SymTabKey &Key) {
    return getTypeFor(Key.Name, Key.CurrScope);
  }

  // Checks if a DeclScope exists in the SymTable for the TabKey.
  bool isDefined(SymTabKey TabKey) {
    return getDeclScopeFor(TabKey.Name, TabKey.CurrScope) != nullptr;
  }

  // Checks if a key exists with the type Type.
  bool containsKeyWithType(PtrType Ty) const {
    return llvm::any_of(llvm::make_second_range(NamesInfo),
                        [Ty](auto &&InfoTy) { return Ty == InfoTy; });
  }

protected:
  llvm::DenseMap<SymTabKey, PtrType> NamesInfo;
};

template <typename Ty> class SymTable;

// Specialization for working with the Paracl language interpreter,
// incorporating a basic type manager for handling created types.
template <> class SymTable<PCLType> : public SymTableBase<PCLType> {
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
      Types.emplace_back(std::make_unique<PCLType>(ID));
    default:
      llvm_unreachable("Unknown type ID");
    }
    assert(!Types.empty());
    return Types.back().get();
  }

private:
  llvm::SmallVector<std::unique_ptr<ValueType>> Types;
};

// To work with code generation, we only need access methods of the SymTableBase
// class, since types in llvm are created by methods of llvm itself, so we
// specialize only by type, without adding anything else.
template <> class SymTable<llvm::Type> : public SymTableBase<llvm::Type> {};

// This base class is responsible for associating a Value with a name, type, and
// other attributes. It provides methods for access and insertion. Important:
// Value insertion and access require a DeclKey, which must be obtained from the
// SymTable class using the appropriate method.
template <typename ValueTy> class ValueManagerBase {
public:
  using ValueType = ValueTy;

  bool linkValueWithName(const SymTabKey &DeclKey, ValueType *Val) {
    auto [_, IsInsert] = NameToValue.insert_or_assign(DeclKey, Val);
    return IsInsert;
  }
  template <typename ValueT = ValueType>
  ValueT *getValueFor(SymTabKey DeclKey) {
    if (!NameToValue.contains(DeclKey))
      return nullptr;
    return static_cast<ValueT *>(NameToValue[DeclKey]);
  }

  bool containsValue(ValueType *Val) const {
    return llvm::find_if(NameToValue, [Val](auto &&MapPair) {
             return MapPair.second == Val;
           }) != NameToValue.end();
  }

protected:
  llvm::DenseMap<SymTabKey, ValueType *> NameToValue;
};

template <typename ValueTy> class ValueManager;

// This is a specialization for the interpreter. It includes the creation and
// storage of PCLValue objects.
template <> class ValueManager<PCLValue> : public ValueManagerBase<PCLValue> {
public:
  template <typename ValueTy, typename... ArgTys>
  ValueTy *createValueFor(const SymTabKey &DeclKey, ArgTys &&...Args) {
    auto ValuePtr = std::make_unique<ValueTy>(std::forward<ArgTys>(Args)...);
    Values.push_back(std::move(ValuePtr));
    auto [InsIt, _] =
        NameToValue.insert_or_assign(DeclKey, Values.back().get());
    return static_cast<ValueTy *>(InsIt->second);
  }

  template <DerivedFromPCLValue ValueTy, typename... ArgTys>
  ValueTy *createValue(ArgTys &&...Args) {
    Values.emplace_back(
        std::make_unique<ValueTy>(std::forward<ArgTys>(Args)...));
    return static_cast<ValueTy *>(Values.back().get());
  }

private:
  llvm::SmallVector<std::unique_ptr<PCLValue>> Values;
};

// This specialization is used for code generation. In addition to the methods
// and fields of the base class, it provides the ability to establish a
// relationship between a Value and its Type. This is necessary because
// sometimes we cannot retrieve the Type for a Value from the SymTable (no key
// is available).
template <>
class ValueManager<llvm::Value> : public ValueManagerBase<llvm::Value> {
public:
  using Type = llvm::Type;

  void setValueTypeLink(ValueType *Val, Type *Ty) {
    ValueToType.insert_or_assign(Val, Ty);
  }

  Type *getTypeFor(ValueType *Val) {
    if (!ValueToType.contains(Val))
      return nullptr;
    return ValueToType[Val];
  }

private:
  llvm::DenseMap<ValueType *, Type *> ValueToType;
};

} // namespace paracl
