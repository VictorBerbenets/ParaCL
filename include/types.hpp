#pragma once

#include <llvm/ADT/SmallVector.h>

namespace paracl {

class PCLType {
public:
  
  enum class TypeID {
    Unknown,
    Int32,
    Array
  };

  PCLType(TypeID ID): ID(ID) {}

  virtual ~PCLType() = default;


  TypeID getTypeID() const noexcept { return ID; }
  bool isInt32Ty() const noexcept { return ID == TypeID::Int32; }
  bool isArrayTy() const noexcept { return ID == TypeID::Array; }
  
protected:
  TypeID ID;
};

class IntegerTy : public PCLType {
public:
  IntegerTy(): PCLType(TypeID::Int32) {}
};

class ArrayTy : public PCLType {
public:
  ArrayTy(): PCLType(TypeID::Array) {} 
  unsigned size() const { return ContainedTypesSize; }

protected:
  unsigned ContainedTypesSize;
  llvm::SmallVector<PCLType *> ContainedTypes;
};

} // namespace paracl
