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

  bool isValid() const { return CurrScope != nullptr; }
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
  static constexpr llvm::StringRef NameForInt32Ty = "int32";
  static constexpr llvm::StringRef NameForUniformArrayTy = "repeat";
  static constexpr llvm::StringRef NameForPresetArrayTy = "array";
  static constexpr llvm::StringRef NameForUnknownTy = "unknown";

public:
  // Standard types of the ParaCL language. In addition to integer types, there
  // are arrays:
  // 1) PresetArray code example : array(1, 2, -1) // 1, 2, -1
  // 2) UniformArray code example: repeat(repeat(1, 5), 10) - a two-dimensional
  //    array of 10 * 5
  enum class TypeID { Int32, UniformArray, PresetArray };

  PCLType(TypeID ID) : ID(ID) {}

  virtual ~PCLType() = default;

  TypeID getTypeID() const noexcept { return ID; }
  bool isInt32Ty() const noexcept { return ID == TypeID::Int32; }
  bool isUniformArrayTy() const noexcept { return ID == TypeID::UniformArray; }
  bool isPresetArrayTy() const noexcept { return ID == TypeID::PresetArray; }
  bool isArrayTy() const noexcept {
    return isUniformArrayTy() || isPresetArrayTy();
  }

  llvm::StringRef getName() const {
    switch (ID) {
    case TypeID::Int32:
      return NameForInt32Ty;
    case TypeID::UniformArray:
      return NameForUniformArrayTy;
    case TypeID::PresetArray:
      return NameForPresetArrayTy;
    default:
      return NameForUnknownTy;
    }
  }

  void print(std::ostream &Os) { Os << getName().str() << '\n'; }

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

  PCLType *getContainedType() const { return ContainedType; }
  std::optional<unsigned> getSize() const { return NumberOfElements; }
  void setSize(unsigned ArrSz) { NumberOfElements = ArrSz; }
  void setContainedType(PCLType *TypeToContain) {
    ContainedType = TypeToContain;
  }

protected:
  // Use optional here because we want to be able to handle simple cases of
  // accessing the boundaries of the array in the ErrorHandler class before
  // executing the program. If the size of the array is known at 'compile time',
  // we set the size and have the opportunity to report an error for example, if
  // we go beyond the boundaries of the array.
  std::optional<unsigned> NumberOfElements = std::nullopt;
  PCLType *ContainedType = nullptr;
};

} // namespace paracl
