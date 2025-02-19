#include <llvm/ADT/ArrayRef.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>

#include "ast_includes.hpp"
#include "codegen_visitor.hpp"

namespace paracl {

using namespace llvm;

CodeGenVisitor::CodeGenVisitor(StringRef ModuleName) : CodeGen(ModuleName) {}

// This visit method for root basic block represents the main module of the
// program (global scope). The main function that launches the program is
// created here (__pcl_start)
void CodeGenVisitor::visit(ast::root_statement_block *RootBlock) {
  auto *PCLStartFunc = CodeGen.createFunction(
      CodeGen.getVoidTy(), Function::ExternalLinkage,
      codegen::IRCodeGenerator::ParaCLStartFuncName, false);
  // Create entry block to start instruction insertion
  auto *EntryBlock =
      BasicBlock::Create(CodeGen.Context, "pcl_entry", PCLStartFunc);
  CodeGen.Builder->SetInsertPoint(EntryBlock);

  // Generating LLVM IR for main objects
  for (auto &&CurStatement : *RootBlock)
    CurStatement->accept(this);

  // Create exit block to end instruction insertion
  CodeGen.createBlockAndLinkWith(CodeGen.Builder->GetInsertBlock(), "pcl_exit");
  // Return void for __pcl_start
  CodeGen.Builder->CreateRetVoid();
}

void CodeGenVisitor::visit(ast::statement_block *StmBlock) {
  auto *CurrBlock = CodeGen.Builder->GetInsertBlock();
  CodeGen.createBlockAndLinkWith(CurrBlock, "local-scope");

  for (auto &&CurStatement : *StmBlock)
    CurStatement->accept(this);
}

void CodeGenVisitor::visit(ast::definition *Def) {}

void CodeGenVisitor::visit(ast::calc_expression *CalcExpr) {}

void CodeGenVisitor::visit(ast::logic_expression *LogicExpr) {}

void CodeGenVisitor::visit(ast::un_operator *UnOper) {}

void CodeGenVisitor::visit(ast::number *Num) {
  CurrVal = ConstantInt::getSigned(CodeGen.getInt32Ty(), Num->get_value());
}

void CodeGenVisitor::visit(ast::variable *Var) {
  setValue(getValueForVar(Var->name()));
}

void CodeGenVisitor::visit(ast::assignment *Assign) {
  Assign->ident_exp()->accept(this);
  NameToValue[Assign->name()] = getCurrValue();
}

void CodeGenVisitor::visit(ast::if_operator *stm) {}

void CodeGenVisitor::visit(ast::while_operator *stm) {}

void CodeGenVisitor::visit(ast::read_expression *stm) {
  auto *ScanType = FunctionType::get(CodeGen.getInt32Ty(), false);
  auto *ScanFunc =
      CodeGen.Mod->getFunction(codegen::IRCodeGenerator::ParaCLScanFuncName);
  auto *ScanRetVal = CodeGen.Builder->CreateCall(ScanType, ScanFunc);
  setValue(ScanRetVal);
}

void CodeGenVisitor::visit(ast::print_function *PrintFuncNode) {
  PrintFuncNode->get()->accept(this);

  SmallVector<llvm::Value *, 1> ArgValue{getCurrValue()};
  SmallVector<Type *, 1> ArgTy{CodeGen.getInt32Ty()};
  auto *PrintType = FunctionType::get(CodeGen.getVoidTy(), ArgTy, false);
  auto *PrintFunc =
      CodeGen.Mod->getFunction(codegen::IRCodeGenerator::ParaCLPrintFuncName);
  CodeGen.Builder->CreateCall(PrintType, PrintFunc, ArgValue);
}

llvm::Value *CodeGenVisitor::getValueForVar(llvm::StringRef ValueName) {
  assert(NameToValue.contains(ValueName));
  return NameToValue[ValueName];
}

void CodeGenVisitor::generateIRCode(ast::root_statement_block *RootBlock,
                                    llvm::raw_ostream &Os) {
  RootBlock->accept(this);
  printIRToOstream(Os);
}

void CodeGenVisitor::printIRToOstream(llvm::raw_ostream &Os) const {
  CodeGen.Mod->print(Os, nullptr);
}

} // namespace paracl
