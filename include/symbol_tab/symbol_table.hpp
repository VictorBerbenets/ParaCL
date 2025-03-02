#pragma once

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/DenseSet.h>
#include <llvm/ADT/Hashing.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/SmallString.h>

#include <iostream>
#include <memory>

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

class PCLValue;

template <typename T>
concept DerivedFromPCLValue = std::derived_from<T, PCLValue>;

template <typename Iter>
concept PCLValuePointerIter =
    std::forward_iterator<Iter> &&
    std::is_pointer_v<typename std::iterator_traits<Iter>::value_type> &&
    DerivedFromPCLValue<
        std::remove_pointer_t<typename std::iterator_traits<Iter>::value_type>>;

class PCLValue {
public:
  virtual ~PCLValue() = default;

  PCLType *getType() { return Ty; }

  virtual PCLValue *clone() const = 0;

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

  IntegerVal *clone() const override { return new IntegerVal(Val, Ty); }

  operator int() const noexcept { return Val; }

private:
  int Val;
};

class ArrayBase: public PCLValue {
protected:

  ArrayBase(PCLType *Ty, unsigned Size): PCLValue(Ty), Size(Size), 
    Data (new PCLValue *[Size]()) {}
  
  ArrayBase(const ArrayBase&) = delete;
  ArrayBase(ArrayBase&& Rhs): PCLValue(std::exchange(Rhs.Ty, nullptr)), Size (std::exchange(Rhs.Size, 0)), Data(std::exchange(Rhs.Data, nullptr)) {}
  ArrayBase &operator=(const ArrayBase&) = delete;
  ArrayBase &operator=(ArrayBase&&Rhs) {
    if (this == std::addressof(Rhs))
      return *this;

    Ty = std::exchange(Rhs.Ty, nullptr);
    Size = std::exchange(Rhs.Size, 0);
    Data = std::exchange(Rhs.Data, nullptr);

    return *this;
  }
  
  ~ArrayBase() override {
    if (Data)
      destroy();
  }

  void destroy() {
    for (auto &&ArrVal : llvm::make_range(begin(), end())) {
      delete ArrVal;
      ArrVal = nullptr;
    }
    delete[] Data;
    Data = nullptr;
    Size = 0;
  }

  bool resizeIfNoData(unsigned NewSize) {
    auto HasData = std::any_of(Data, Data + Size, [](auto *CurrVal) { return CurrVal != nullptr; });
    if (HasData)
      return false;

    delete [] Data;
    Size = NewSize;
    Data = new PCLValue*[Size]();

    return true;
  }

public: 

  PCLValue *operator[](unsigned Id) { return Data[Id]; }

  PCLValue **begin() { return Data; }
  PCLValue **begin() const { return Data; }
  PCLValue **end() {
    assert(Size);
    return Data + Size;
  }
  PCLValue **end() const {
    assert(Size);
    return Data + Size;
  }

  void copy(PCLValue **DestIter) {
    std::transform(begin(), end(), DestIter, [](auto *CurrVal) { return CurrVal->clone(); });
  }
  
  unsigned getSize() const noexcept { return Size; }

protected:
  unsigned Size = 0;
  PCLValue **Data = nullptr;
};

class PresetArrayVal : public ArrayBase {
  using TypeID = PCLType::TypeID;
public:
  template <PCLValuePointerIter Iter>
  PresetArrayVal(Iter Begin, Iter End, PCLType *Ty): ArrayBase(Ty, std::distance(Begin, End)) {
    resizeForConcatIfNeed(Begin, End);
    ArrayBase *ArrPtr = nullptr;
    for (unsigned ArrIndex = 0; auto *AssignVal : llvm::make_range(Begin, End)) {
      auto *Type = AssignVal->getType();
      switch(Type->getTypeID()) {
        case TypeID::UniformArray:
            [[fallthrough]];
        case TypeID::PresetArray:
            ArrPtr = static_cast<ArrayBase*>(AssignVal);
            ArrPtr->copy(std::addressof(Data[ArrIndex]));
            ArrIndex += ArrPtr->getSize();
            break;
        case TypeID::Int32:
            Data[ArrIndex] = AssignVal->clone();
            ArrIndex ++;
            break;
        case TypeID::Unknown:
            [[fallthrough]];
        default:
            llvm_unreachable("Unsupported type for preset array");
      } 
    }
  }

  PresetArrayVal *clone() const override { return new PresetArrayVal(Data, Data + Size, Ty); };

private:

  template <PCLValuePointerIter Iter>
  void resizeForConcatIfNeed(Iter Begin, Iter End) {
    unsigned NewSize = 0;
    std::for_each(Begin, End, [&](auto *CurrVal) {
        auto *Type = CurrVal->getType();
        if (Type->isArrayTy()) {
          // Avoiding arrays of size 0
          if (auto Sz = static_cast<ArrayBase*>(CurrVal)->getSize(); Sz)
            NewSize += Sz;
        } else {
          NewSize ++;
        } });

    if (NewSize != Size) {
      resizeIfNoData(NewSize);
    }
  }
};

class UniformArrayVal : public ArrayBase {
public:
  UniformArrayVal(PCLValue *Initer, unsigned Size, PCLType *Ty)
      : ArrayBase(Ty, Size), Initer(Initer)
         {
    construct();
  }

  UniformArrayVal(const UniformArrayVal &Rhs) = delete;
  UniformArrayVal(UniformArrayVal &&Rhs): ArrayBase(std::move(Rhs)), Initer(std::exchange(Rhs.Initer, nullptr)) {}
  UniformArrayVal &operator=(const UniformArrayVal &Rhs) = delete;
  UniformArrayVal &operator=(UniformArrayVal &&Rhs) {
    if (this == std::addressof(Rhs))
      return *this;

    Ty = std::exchange(Rhs.Ty, nullptr);
    Size = std::exchange(Rhs.Size, 0);
    Data = std::exchange(Rhs.Data, nullptr);
    Initer = std::exchange(Rhs.Initer, nullptr); 

    return *this;
  }

  ~UniformArrayVal() override {
    destroy();
    Initer = nullptr;
   }
 
  UniformArrayVal *clone() const override { return new UniformArrayVal(Initer, Size, Ty); }

  PCLValue *getIniter() noexcept { return Initer; }

private:
  void construct() {
    assert(Initer);
  std::cout << "LINE = " << __LINE__ << '\n';
    for (auto &&ArrVal : llvm::make_range(begin(), end())) {
      ArrVal = Initer->clone();
    }
  std::cout << "LINE = " << __LINE__ << '\n';
  }

  PCLValue *Initer = nullptr;
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
