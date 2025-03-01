#pragma once

#include <llvm/ADT/StringRef.h>

#include <map>
#include <utility>

#include "codegen.hpp"
#include "symbol_table.hpp"
#include "visitor.hpp"

namespace paracl {

class CodeGenVisitor : public base_visitor {
public:
  CodeGenVisitor(SymTable &SymTbl, ValueManager &ValManager,
                 llvm::StringRef ModuleName);

  virtual void visit(ast::root_statement_block *stm);
  virtual void visit(ast::definition *stm);

  void visit(ast::InitListArray *InitListArr) override {}
  void visit(ast::ArrayAccess *ArrAccess) override {}
  void visit(ast::UndefVar *UndVar) override {}
  void visit(ast::Array *Arr) override {}

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

  void setCurrValue(llvm::Value *Value) noexcept { CurrVal = Value; }

  llvm::Value *getValueForVar(llvm::StringRef VarName);

  void printIRToOstream(llvm::raw_ostream &Os) const;

  SymTable &SymTbl;
  ValueManager &ValManager;
  llvm::DenseMap<llvm::StringRef, llvm::Value *> NameToValue;
  codegen::IRCodeGenerator CodeGen;
  llvm::Value *CurrVal;
};

} // namespace paracl
