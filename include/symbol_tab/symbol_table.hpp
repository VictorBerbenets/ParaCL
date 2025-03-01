#pragma once

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/Hashing.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/DenseSet.h>

#include <memory>
#include <iostream>

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

template <> struct DenseMapInfo<paracl::SymTabKey> {
  static inline paracl::SymTabKey getEmptyKey() {
    return {StringRef(""), nullptr};
  }

  static inline paracl::SymTabKey getTombstoneKey() {
    return {StringRef(""), nullptr};
  }

  static unsigned getHashValue(const paracl::SymTabKey &TabKey) {
    return llvm::hash_value(TabKey.Name.str()) +
           llvm::hash_value(TabKey.CurrScope);
  }

  static bool isEqual(const paracl::SymTabKey &LHS,
                      const paracl::SymTabKey &RHS) {
    return LHS.CurrScope == RHS.CurrScope && LHS.Name == RHS.Name;
  }
};

} // namespace llvm

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
      Types.try_emplace(ID, std::make_unique<IntegerTy>());
      break;
    case TypeID::Array:
      Types.try_emplace(ID, std::make_unique<ArrayTy>());
      break;
    default:
      Types.try_emplace(TypeID::Unknown, std::make_unique<PCLType>(TypeID::Unknown));
    }
    return Types[ID].get();
  }

  template <typename... ArgsTy>
  bool tryDefine(const SymbNameType &Name, ast::statement_block *CurrScope,
                 ArgsTy &&...Args) {
    if (isDefined({Name, CurrScope}))
      return false;

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

class PCLValue {
public:
  virtual ~PCLValue() = default;

  PCLType *getType() { return Ty; }

protected:
  PCLValue(PCLType *Ty) : Ty(Ty) {}

  PCLType *Ty = nullptr;
};

class IntegerVal : public PCLValue {
public:
  IntegerVal(int Val, PCLType *Ty) : PCLValue(Ty), Val(Val) {}

  int getValue() const noexcept { return Val; }
  void setValue(IntegerVal *NewValue) noexcept {
    assert(NewValue);
    Val = NewValue->getValue();
  }

  operator int() const noexcept { return Val; }

private:
  int Val;
};

class ArrayVal : public PCLValue {
public:
  ArrayVal(PCLValue *Initer, IntegerVal *Size, PCLType *Ty) : PCLValue(Ty),
      Initer(Initer), Size(Size), Data(new PCLValue*[Size->getValue()]) {
    assert(Initer && Size);
    std::cout << "Array Address in constructor = " << std::hex << this << std::dec << '\n';
    std::cout << "Initer Address in constructor = " << std::hex << Initer << std::dec << '\n';
    std::cout << "Size in constructor = " << Size->getValue() << std::dec << '\n';
    for (auto Id = 0; Id < Size->getValue(); ++Id) {
      if (Initer->getType()->isInt32Ty()) {
      }
        Data[Id] = new IntegerVal(*static_cast<IntegerVal*>(Initer));
      if (Initer->getType()->isArrayTy()) {
        Data[Id] = new ArrayVal(*static_cast<ArrayVal*>(Initer));
      }
    }
  }
 
  ArrayVal(PCLType *Ty, IntegerVal *Size): PCLValue(Ty) {}

  ArrayVal(const ArrayVal &Rhs): PCLValue(Rhs.Ty), Initer(Rhs.Initer), Size(Rhs.Size), Data (new PCLValue*[Size->getValue()]) {
    std::cout << "Array Address in COPY constructor = " << std::hex << this << std::dec << '\n';
    std::cout << "Initer Address in COPY constructor = " << std::hex << Rhs.Initer << std::dec << '\n';
    std::cout << "Size in COPY constructor = " << Size->getValue() << std::dec << '\n';
    std::cout << "LINE = " << __LINE__ << '\n';
    assert(Initer);
    auto *InitType = Initer->getType();
    assert(InitType);
    for (auto Id = 0; Id < Size->getValue(); ++Id) {
      if (InitType->isInt32Ty()) {
        Data[Id] = new IntegerVal(*static_cast<IntegerVal*>(Initer));
      }
      if (InitType->isArrayTy()) {
        Data[Id] = new ArrayVal(*static_cast<ArrayVal*>(Initer));
      }
    }
 
  }

  const ArrayVal &operator=(const ArrayVal &Rhs) {
    if (this == std::addressof(Rhs))
      return *this;

    auto Copy = Rhs;
    swap(Copy);
    return *this;
  }

  void swap(const ArrayVal &Rhs) {

  }

  ~ArrayVal() {
    std::cout << "Destructor for array = " << std::hex << this << std::dec << std::endl;
    if (Data)
      destroy();
  }

  void destroy() {
    for (auto Id = 0; Id < Size->getValue(); ++Id) {
        delete Data[Id];
        Data[Id] = nullptr;
    }
    delete [] Data;
    Data = nullptr;
    Initer = nullptr;
    Size = nullptr;
  }

  PCLValue *operator[](unsigned Id) {
    return Data[Id];
  }

  PCLValue *getIniter() noexcept { return Initer; }
  IntegerVal *getSize() noexcept { return Size; }

private:
  PCLValue *Initer = nullptr;
  IntegerVal *Size = nullptr;
  PCLValue **Data;
};

template <typename T>
concept DerivedFromPCLVal = std::derived_from<T, PCLValue>;

class ValueManager final {
public:
  template <DerivedFromPCLVal ValueTy, typename... ArgTys>
  PCLValue *createValueFor(SymTabKey &&SymKey, ArgTys &&...Args) {
    auto ValuePtr = std::make_unique<ValueTy>(std::forward<ArgTys>(Args)...);
    Values.push_back(std::move(ValuePtr));
    auto [InsIt, _] = NameToValue.insert_or_assign(std::forward<SymTabKey>(SymKey), Values.back().get());
    return InsIt->second;
  }

  bool linkValueWithName(SymTabKey &&SymKey, PCLValue *Val) {
    auto [_, IsInsert] = NameToValue.insert_or_assign(std::forward<SymTabKey>(SymKey), Val); 
    return IsInsert;
  }

  template <DerivedFromPCLVal ValueTy, typename... ArgTys>
  PCLValue *createValue(ArgTys &&...Args) {
    Values.emplace_back(
        std::make_unique<ValueTy>(std::forward<ArgTys>(Args)...));
    return Values.back().get();
  }

  PCLValue *getValueFor(SymTabKey SymKey) {
    if (!NameToValue.contains(SymKey))
      return nullptr;
    return NameToValue[SymKey];
  }

  friend class driver;

private:
  llvm::DenseMap<SymTabKey, PCLValue*> NameToValue;
  llvm::SmallVector<std::unique_ptr<PCLValue>> Values;
};

} // namespace paracl
