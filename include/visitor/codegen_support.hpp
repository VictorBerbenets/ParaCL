#pragma once

#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>

#include <concepts>
#include <memory>

#include "values.hpp"

namespace paracl {
namespace codegen {

using namespace llvm;

template <typename ConstTy>
concept DerivedFromLLVMConstant = std::derived_from<ConstTy, Constant>;

inline void fillArrayWithData(IRBuilder<> &Builder, Value *ArrPtr,
                                Type *DataTy, ArrayRef<Value *> Data) {
  for (unsigned Id = 0; Id < Data.size(); ++Id) {
    auto *GEPPtr =
        Builder.CreateGEP(DataTy, ArrPtr, ConstantInt::get(DataTy, Id));
    Builder.CreateStore(Data[Id], GEPPtr);
  }
}
  
inline ConstantInt *isConstantInt(Value *Val) {
  return dyn_cast<ConstantInt>(Val);
}

inline bool isConstantData(ArrayRef<Value *> Data) {
  return all_of(Data, [](auto *Val) { return isConstantInt(Val) != nullptr; });

}
  
template <DerivedFromLLVMConstant ConstTy = Constant>
ConstTy *isCompileTimeConstant(Value *Val) {
  return dyn_cast<ConstTy>(Val);
}
  
template <DerivedFromLLVMConstant ConstType = Constant>
std::optional<SmallVector<ConstType *>>
tryConvertDataToConstant(ArrayRef<Value *> Data) {
  SmallVector<ConstType *> ConstData;
  ConstData.reserve(Data.size());
  if (!isConstantData(Data))
    return {};

  llvm::transform(Data, std::back_inserter(ConstData), [](auto *Val) {
    auto *ConstVal = dyn_cast<ConstType>(Val);
    assert(ConstVal);
    return ConstVal;
  });
  return ConstData;
}

// An auxiliary structure for recursively collecting data from an entire array
// and preparing for its creation
struct ArrayInfo final {
  SmallVector<Value *> Sizes;
  SmallVector<Value *> Data;

  bool isConstant() const {
    return isConstantData(Data) && isConstantData(Sizes);
  }

  void pushSize(Value *Sz) { Sizes.push_back(Sz); }

  void pushData(Value *Dat) { Data.push_back(Dat); }

  template <std::input_iterator InputIt>
  void pushData(InputIt Begin, InputIt End) {
    Data.insert(Data.end(), Begin, End);
  }

  void clearSize() { Sizes.clear(); }

  void clearData() { Data.clear(); }

  void clear() {
    clearSize();
    clearData();
  }

  static Value *calculateSize(IRBuilder<> &Builder, IntegerType *DataTy,
                              ArrayRef<Value *> Data) {
    if (auto OptData = tryConvertDataToConstant<ConstantInt>(Data);
        OptData.has_value())
      return calculateSize(DataTy, OptData.value());

    Value *ArrSize = ConstantInt::get(DataTy, 1);
    llvm::for_each(
        Data, [&](auto *Sz) { ArrSize = Builder.CreateMul(ArrSize, Sz); });
    return ArrSize;
  }

  static ConstantInt *calculateSize(IntegerType *DataTy,
                                    ArrayRef<ConstantInt *> Data) {
    auto *ArrSize = ConstantInt::get(DataTy, 1);
    llvm::for_each(Data, [&ArrSize](auto *Sz) {
      ArrSize = dyn_cast<ConstantInt>(ConstantExpr::getMul(ArrSize, Sz));
      assert(ArrSize);
    });
    return ArrSize;
  }
};

class ArrayManager final {
public:
  ArrayManager() = default;
  
  bool containsArrayInfo(ArrayInfo *ArrInfo) const {
    return llvm::find_if(ArrayInfoStor, [ArrInfo](auto &&UnPtr) {
          return UnPtr.get() == ArrInfo; 
        }) != ArrayInfoStor.end(); 
  }
  bool containsArrayPtr(Value *ArrPtr) const {
    return ArrayInfoMap.contains(ArrPtr); 
  }

  ArrayInfo &create() {
    ArrayInfoStor.emplace_back(std::make_unique<ArrayInfo>());
    return *ArrayInfoStor.back().get();
  }

  ArrayInfo &operator[](Value *ArrPtr) {
    return ArrayInfoMap[ArrPtr];
  }

private:
  DenseMap<Value*, ArrayInfo&> ArrayInfoMap;
  SmallVector<std::unique_ptr<ArrayInfo>> ArrayInfoStor;
};

} // namespace codegen
} // namespace paracl
