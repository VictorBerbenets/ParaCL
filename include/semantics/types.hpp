#pragma once

#include <llvm/ADT/Hashing.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/SmallVector.h>

namespace paracl {

namespace ast {
class statement_block;
} // namespace ast

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

class PCLType {
public:
  enum class TypeID { Unknown, Int32, UniformArray, PresetArray };

  PCLType(TypeID ID) : ID(ID) {}

  virtual ~PCLType() = default;

  TypeID getTypeID() const noexcept { return ID; }
  bool isInt32Ty() const noexcept { return ID == TypeID::Int32; }
  bool isUniformArrayTy() const noexcept { return ID == TypeID::UniformArray; }
  bool isPresetArrayTy() const noexcept { return ID == TypeID::PresetArray; }
  bool isArrayTy() const noexcept {
    return isUniformArrayTy() || isPresetArrayTy();
  }

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
