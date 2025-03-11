#pragma once

#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Casting.h>

#include <concepts>

#include "codegen.hpp"
#include "semantic_context.hpp"
#include "visitor.hpp"

namespace paracl {

template <typename ConstTy>
concept DerivedFromLLVMConstant = std::derived_from<ConstTy, llvm::Constant>;

class CodeGenVisitor : public VisitorBase {
  struct ArrayInfo {
    llvm::SmallVector<llvm::Value *> Sizes;
    llvm::SmallVector<llvm::Value *> Data;

    void clear() {
      Sizes.clear();
      Data.clear();
    }
  };

public:
  CodeGenVisitor(llvm::StringRef ModuleName);

  virtual void visit(ast::root_statement_block *stm);
  virtual void visit(ast::definition *stm);

  void visit(ast::ArrayHolder *ArrStore) override;
  void visit(ast::ArrayAccessAssignment *Arr) override;
  void visit(ast::PresetArray *PresetArr) override;
  void visit(ast::ArrayAccess *ArrAccess) override;
  void visit(ast::UniformArray *UnifArr) override;

  void visit(ast::calc_expression *stm) override;
  void visit(ast::un_operator *stm) override;
  void visit(ast::logic_expression *stm) override;
  void visit(ast::number *stm) override;
  void visit(ast::variable *stm) override;
  void visit(ast::assignment *stm) override;
  void visit(ast::read_expression *stm) override;
  void visit(ast::statement_block *stm) override;
  void visit(ast::if_operator *stm) override;
  void visit(ast::while_operator *stm) override;
  void visit(ast::print_function *stm) override;

  // Generate LLVM IR and write it to Os
  void generateIRCode(ast::root_statement_block *RootBlock,
                      llvm::raw_ostream &Os);

private:
  llvm::Value *getCurrValue() const noexcept { return CurrVal; }
  llvm::Value *getValueAfterAccept(ast::statement *Stm);

  void setCurrValue(llvm::Value *Value) noexcept { CurrVal = Value; }

  llvm::Value *getValueForVar(llvm::StringRef VarName);
  llvm::Type *getValueType(llvm::Value *Val);

  void printIRToOstream(llvm::raw_ostream &Os) const;

  template <DerivedFromLLVMConstant ConstTy = llvm::Constant>
  ConstTy *isCompileTimeConstant(llvm::Value *Val) const {
    return dyn_cast<ConstTy>(Val);
  }

  llvm::DenseMap<llvm::StringRef, llvm::Value *> NameToValue;
  llvm::DenseMap<llvm::Value *, llvm::Type *> ValueToType;
  codegen::IRCodeGenerator CodeGen;
  llvm::Value *CurrVal;
};

} // namespace paracl
