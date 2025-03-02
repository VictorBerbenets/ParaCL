#pragma once

#include <llvm/ADT/SmallVector.h>

namespace paracl {

class PCLType {
public:
  enum class TypeID { Unknown, Int32, UniformArray, PresetArray };

  PCLType(TypeID ID) : ID(ID) {}

  virtual ~PCLType() = default;

  TypeID getTypeID() const noexcept { return ID; }
  bool isInt32Ty() const noexcept { return ID == TypeID::Int32; }
  bool isUniformArrayTy() const noexcept { return ID == TypeID::UniformArray; }
  bool isPresetArrayTy() const noexcept { return ID == TypeID::PresetArray; }
  bool isArrayTy() const noexcept { return isUniformArrayTy() || isPresetArrayTy(); }

protected:
  TypeID ID;
};

inline bool operator==(PCLType Lhs, PCLType Rhs) {
  return Lhs.getTypeID() == Rhs.getTypeID();
}

class IntegerTy : public PCLType {
public:
  IntegerTy(TypeID IntID) : PCLType(IntID) {}
};

class ArrayTy : public PCLType {
public:
  ArrayTy(TypeID ArrID) : PCLType(ArrID) {}
  unsigned size() const { return ContainedTypesSize; }

protected:
  unsigned ContainedTypesSize;
  llvm::SmallVector<PCLType *> ContainedTypes;
};

} // namespace paracl
