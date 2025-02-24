#pragma once

#include <llvm/ADT/SmallVector.h>

namespace paracl {

class PCLType {
public:
  
  enum class TypeID {
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
  unsigned ContainedTypesSize;
  llvm::SmallVector<PCLType *> ContainedTypes;
};

class IntegerTy : public PCLType {
public:
  IntegerTy(): PCLType(TypeID::Int32) {}
};

class IArrayTy : public PCLType {
public:

  IntegerTy *getIndexNum() { return IndexNum; }
  void setIndexNum(IntegerTy *IndexN) { IndexNum = IndexN; }

protected:
  IntegerTy *IndexNum = nullptr;
};

class InitListArray : public IArrayTy {

};

} // namespace paracl
