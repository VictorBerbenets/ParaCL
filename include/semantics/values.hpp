#pragma once

#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/Value.h>

#include <concepts>
#include <iostream>
#include <iterator>
#include <type_traits>
#include <utility>

#include "types.hpp"

namespace paracl {

class ValueWrapper;
class PCLValue;

template <typename T>
concept DerivedFromValueWrapper = std::derived_from<T, ValueWrapper>;

template <typename T>
concept DerivedFromPCLValue = std::derived_from<T, PCLValue>;

template <typename Iter>
concept PCLValuePointerIter =
    std::forward_iterator<Iter> &&
    std::is_pointer_v<typename std::iterator_traits<Iter>::value_type> &&
    DerivedFromPCLValue<
        std::remove_pointer_t<typename std::iterator_traits<Iter>::value_type>>;

template <typename T>
concept IsPureType = std::is_same_v<std::decay_t<T>, T>;

class ValueWrapper {
protected:
  ValueWrapper() = default;

public:
  virtual ~ValueWrapper() = default;
};

// We usually only need wrapper for one value class. Wrapper for visitor return
// type inherits from this class to get a useful interface
template <IsPureType T> class ValueWrapperInterface {
protected:
  using WrapperInterfaceTy = ValueWrapperInterface<T>;

public:
  using ValueType = T *;

  ValueWrapperInterface(ValueType Val) : Val(Val) {}

  ValueType get() noexcept { return Val; }
  template <std::derived_from<T> CastTo> CastTo *getAs() {
    assert(Val);
    return static_cast<CastTo *>(Val);
  }

  void set(ValueType ValToSet) noexcept { Val = ValToSet; }

  bool isNull() const noexcept { return Val == nullptr; }

  ValueType operator->() { return Val; }

  operator ValueType() { return Val; }

protected:
  ValueType Val;
};

class PCLValue {
public:
  virtual ~PCLValue() = default;

  virtual PCLType *getType() const { return Ty; }

  virtual PCLValue *clone() const = 0;

  virtual void print(std::ostream &Os) const = 0;

protected:
  PCLValue(PCLType *Ty) : Ty(Ty) {}

  PCLType *Ty = nullptr;
};

class IntegerVal : public PCLValue {
public:
  IntegerVal(int Val, IntegerTy *Ty) : PCLValue(Ty), Val(Val) {}

  IntegerTy *getType() const override { return static_cast<IntegerTy *>(Ty); }

  void print(std::ostream &Os) const override { Os << Val << '\n'; }

  int getValue() const noexcept { return Val; }
  void setValue(IntegerVal *NewValue) noexcept {
    assert(NewValue);
    Val = NewValue->getValue();
  }

  IntegerVal *clone() const override {
    return new IntegerVal(Val, static_cast<IntegerTy *>(Ty));
  }

  operator int() const noexcept { return Val; }

private:
  int Val;
};

class ArrayBase : public PCLValue {
protected:
  ArrayBase(ArrayTy *Ty, unsigned Size)
      : PCLValue(Ty), Size(Size), Data(new PCLValue *[Size]()) {
    Ty->setSize(Size);
  }

  ArrayBase(const ArrayBase &) = delete;
  ArrayBase(ArrayBase &&Rhs)
      : PCLValue(std::exchange(Rhs.Ty, nullptr)),
        Size(std::exchange(Rhs.Size, 0)),
        Data(std::exchange(Rhs.Data, nullptr)) {}
  ArrayBase &operator=(const ArrayBase &) = delete;
  ArrayBase &operator=(ArrayBase &&Rhs) {
    if (this == std::addressof(Rhs))
      return *this;

    Ty = std::exchange(Rhs.Ty, nullptr);
    Size = std::exchange(Rhs.Size, 0);
    Data = std::exchange(Rhs.Data, nullptr);

    return *this;
  }

  ~ArrayBase() override {
    if (Data) {
      destroy();
    }
  }

  ArrayTy *getType() const override { return static_cast<ArrayTy *>(Ty); }

  void print(std::ostream &Os) const override {
    for (unsigned Id = 0; Id < Size; ++Id) {
      assert(Data[Id]);
      Data[Id]->print(Os);
    }
  }

  bool resizeIfNoData(unsigned NewSize) {
    auto HasData = std::any_of(
        Data, Data + Size, [](auto *CurrVal) { return CurrVal != nullptr; });
    if (HasData)
      return false;

    delete[] Data;
    Size = NewSize;
    Data = new PCLValue *[Size]();

    return true;
  }

public:
  PCLValue *operator[](unsigned Id) { return Data[Id]; }

  PCLValue **begin() { return Data; }
  PCLValue **begin() const { return Data; }
  PCLValue **end() { return Data + Size; }
  PCLValue **end() const { return Data + Size; }

  void copy(PCLValue **DestIter) {
    std::transform(begin(), end(), DestIter,
                   [](auto *CurrVal) { return CurrVal->clone(); });
  }

  unsigned getSize() const noexcept { return Size; }

  void destroy() {
    for (auto &&ArrVal : llvm::make_range(begin(), end())) {
      delete ArrVal;
      ArrVal = nullptr;
    }
    delete[] Data;
    Data = nullptr;
    Size = 0;
  }

protected:
  unsigned Size = 0;
  PCLValue **Data = nullptr;
};

class PresetArrayVal : public ArrayBase {
  using TypeID = PCLType::TypeID;

public:
  template <PCLValuePointerIter Iter>
  PresetArrayVal(Iter Begin, Iter End, ArrayTy *Ty)
      : ArrayBase(Ty, std::distance(Begin, End)) {
    resizeForConcatIfNeed(Begin, End);
    ArrayBase *ArrPtr = nullptr;
    for (unsigned ArrIndex = 0;
         auto *AssignVal : llvm::make_range(Begin, End)) {
      assert(AssignVal);
      auto *Type = AssignVal->getType();
      switch (Type->getTypeID()) {
      case TypeID::UniformArray:
      case TypeID::PresetArray:
        ArrPtr = static_cast<ArrayBase *>(AssignVal);
        ArrPtr->copy(std::addressof(Data[ArrIndex]));
        ArrIndex += ArrPtr->getSize();
        break;
      case TypeID::Int32:
        Data[ArrIndex] = AssignVal->clone();
        ArrIndex++;
        break;
      default:
        llvm_unreachable("Unsupported type for preset array");
      }
    }
    getType()->setSize(Size);
  }

  PresetArrayVal *clone() const override {
    return new PresetArrayVal(Data, Data + Size, getType());
  };

private:
  // One of the initialization values of the Preset array may be another array.
  // In this case, we need to calculate the size of the resulting array in
  // advance.
  template <PCLValuePointerIter Iter>
  void resizeForConcatIfNeed(Iter Begin, Iter End) {
    unsigned NewSize = 0;
    std::for_each(Begin, End, [&](auto *CurrVal) {
      auto *Type = CurrVal->getType();
      if (Type->isArrayTy()) {
        // Avoiding arrays of size 0
        if (auto Sz = static_cast<ArrayBase *>(CurrVal)->getSize(); Sz)
          NewSize += Sz;
      } else {
        NewSize++;
      }
    });

    if (NewSize != Size) {
      resizeIfNoData(NewSize);
    }
  }
};

class UniformArrayVal : public ArrayBase {
public:
  UniformArrayVal(PCLValue *Initer, unsigned Size, ArrayTy *Ty)
      : ArrayBase(Ty, Size), Initer(Initer) {
    Ty->setSize(Size);
    assert(Initer);
    Ty->setContainedType(Initer->getType());
    construct();
  }

  UniformArrayVal(const UniformArrayVal &Rhs) = delete;
  UniformArrayVal(UniformArrayVal &&Rhs)
      : ArrayBase(std::move(Rhs)), Initer(std::exchange(Rhs.Initer, nullptr)) {}
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

  UniformArrayVal *clone() const override {
    return new UniformArrayVal(Initer, Size, getType());
  }

  PCLValue *getIniter() noexcept { return Initer; }

private:
  void construct() {
    assert(Initer);
    for (auto &&ArrVal : llvm::make_range(begin(), end())) {
      ArrVal = Initer->clone();
    }
  }

  PCLValue *Initer = nullptr;
};

} // namespace paracl
