#include <llvm/ADT/ArrayRef.h>
#include <llvm/IR/BasicBlock.h>

#include "ast_includes.hpp"
#include "codegen_visitor.hpp"

namespace paracl {

using namespace llvm;

CodeGenVisitor::CodeGenVisitor(StringRef ModuleName) : CodeGen(ModuleName) {}

// This visit method for root basic block represents the main module of the
// program (global scope). The main function that launches the program is
// created here
// (__pcl_start)
void CodeGenVisitor::visit(ast::root_statement_block *RootBlock) {
  auto *FuncType = FunctionType::get(CodeGen.getVoidTy(), false);
  auto *PCLStartFunc = Function::Create(
      FuncType, Function::ExternalLinkage,
      codegen::IRCodeGenerator::ParaCLStartFuncName, CodeGen.Mod.get());
  // Create entry block to start instruction insertion
  auto *EntryBlock =
      BasicBlock::Create(CodeGen.Context, "pcl_entry", PCLStartFunc);
  CodeGen.Builder->SetInsertPoint(EntryBlock);

  // Generating LLVM IR for main objects
  for (auto &&CurStatement : *RootBlock)
    CurStatement->accept(this);

  // Create exit block to end instruction insertion
  auto *EndBlock =
      BasicBlock::Create(CodeGen.Context, "pcl_exit", PCLStartFunc);
  CodeGen.Builder->CreateBr(EndBlock);
  CodeGen.Builder->SetInsertPoint(EndBlock);
  // Return void for __pcl_start
  CodeGen.Builder->CreateRetVoid();
}

void CodeGenVisitor::visit(ast::statement_block *StmBlock) {}

void CodeGenVisitor::visit(ast::definition *Def) {}

void CodeGenVisitor::visit(ast::calc_expression *CalcExpr) {}

void CodeGenVisitor::visit(ast::logic_expression *LogicExpr) {}

void CodeGenVisitor::visit(ast::un_operator *UnOper) {}

void CodeGenVisitor::visit(ast::number *Num) {}

void CodeGenVisitor::visit(ast::variable *Var) {}

void CodeGenVisitor::visit(ast::assignment *Assign) {}

void CodeGenVisitor::visit(ast::if_operator *stm) {}

void CodeGenVisitor::visit(ast::while_operator *stm) {}

void CodeGenVisitor::visit(ast::read_expression *stm) {}

void CodeGenVisitor::visit(ast::print_function *stm) {}

void CodeGenVisitor::generateIRCode(ast::root_statement_block *RootBlock,
                                    llvm::raw_ostream &Os) {
  RootBlock->accept(this);
  printIRToOstream(Os);
}

void CodeGenVisitor::printIRToOstream(llvm::raw_ostream &Os) const {
  CodeGen.Mod->print(Os, nullptr);
}

} // namespace paracl
