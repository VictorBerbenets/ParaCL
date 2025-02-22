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
  for (auto &&CurStatement : *StmBlock)
    CurStatement->accept(this);
}

void CodeGenVisitor::visit(ast::definition *) { /*unused*/ }

void CodeGenVisitor::visit(ast::calc_expression *CalcExpr) {
  CalcExpr->left()->accept(this);
  auto *Lhs = getCurrValue();
  CalcExpr->right()->accept(this);
  auto *Rhs = getCurrValue();

  switch (CalcExpr->type()) {
  case ast::CalcOp::ADD:
    setCurrValue(CodeGen.Builder->CreateAdd(Lhs, Rhs));
    break;
  case ast::CalcOp::SUB:
    setCurrValue(CodeGen.Builder->CreateSub(Lhs, Rhs));
    break;
  case ast::CalcOp::MUL:
    setCurrValue(CodeGen.Builder->CreateMul(Lhs, Rhs));
    break;
  case ast::CalcOp::PERCENT:
    setCurrValue(CodeGen.Builder->CreateSRem(Lhs, Rhs));
    break;
  case ast::CalcOp::DIV:
    setCurrValue(CodeGen.Builder->CreateSDiv(Lhs, Rhs));
    break;
  default:
    llvm_unreachable("unrecognized type for calc_expression");
  }
}

void CodeGenVisitor::visit(ast::logic_expression *LogicExpr) {
  LogicExpr->left()->accept(this);
  auto *Lhs = getCurrValue();
  LogicExpr->right()->accept(this);
  auto *Rhs = getCurrValue();

  switch (LogicExpr->type()) {
  case ast::LogicOp::LOGIC_AND:
    setCurrValue(CodeGen.Builder->CreateAnd(Lhs, Rhs));
    break;
  case ast::LogicOp::LOGIC_OR:
    setCurrValue(CodeGen.Builder->CreateOr(Lhs, Rhs));
    break;
  case ast::LogicOp::LESS:
    setCurrValue(CodeGen.Builder->CreateICmpSLT(Lhs, Rhs));
    break;
  case ast::LogicOp::LESS_EQ:
    setCurrValue(CodeGen.Builder->CreateICmpSLE(Lhs, Rhs));
    break;
  case ast::LogicOp::GREATER:
    setCurrValue(CodeGen.Builder->CreateICmpSGT(Lhs, Rhs));
    break;
  case ast::LogicOp::GREATER_EQ:
    setCurrValue(CodeGen.Builder->CreateICmpSGE(Lhs, Rhs));
    break;
  case ast::LogicOp::EQ:
    setCurrValue(CodeGen.Builder->CreateICmpEQ(Lhs, Rhs));
    break;
  case ast::LogicOp::NEQ:
    setCurrValue(CodeGen.Builder->CreateICmpNE(Lhs, Rhs));
    break;
  default:
    llvm_unreachable("unrecognized type for logic expression");
  }
}

void CodeGenVisitor::visit(ast::un_operator *UnOper) {
  UnOper->arg()->accept(this);

  switch (UnOper->type()) {
  case ast::UnOp::PLUS:
    // Do nothing with the value
    setCurrValue(getCurrValue());
    break;
  case ast::UnOp::MINUS:
    setCurrValue(CodeGen.Builder->CreateNeg(getCurrValue()));
    break;
  case ast::UnOp::NEGATE:
    setCurrValue(CodeGen.Builder->CreateICmpEQ(
        getCurrValue(), CodeGen.createConstantSInt32(0)));
    break;
  default:
    llvm_unreachable("unrecognized type for un_operator");
  }
}

void CodeGenVisitor::visit(ast::number *Num) {
  setCurrValue(CodeGen.createConstantSInt32(Num->get_value()));
}

void CodeGenVisitor::visit(ast::variable *Var) {
  auto *Value = getValueForVar(Var->name());
  auto *AllocTy = static_cast<AllocaInst *>(Value)->getAllocatedType();
  auto *Load = CodeGen.Builder->CreateLoad(AllocTy, Value, Var->name());
  setCurrValue(Load);
}

void CodeGenVisitor::visit(ast::assignment *Assign) {
  Assign->ident_exp()->accept(this);
  if (!NameToValue.contains(Assign->name())) {
    auto *Alloca = CodeGen.Builder->CreateAlloca(CodeGen.getInt32Ty(), nullptr,
                                                 Assign->name());
    CodeGen.Builder->CreateStore(getCurrValue(), Alloca);
    NameToValue[Assign->name()] = Alloca;
  } else {
    CodeGen.Builder->CreateStore(getCurrValue(), NameToValue[Assign->name()]);
  }
}

void CodeGenVisitor::visit(ast::if_operator *If) {
  auto *CurrBlock = CodeGen.createBlockAndLinkWith(
      CodeGen.Builder->GetInsertBlock(), "if-cond");
  // Evaluate condition
  If->condition()->accept(this);
  auto *CondValue = CodeGen.createCondValueIfNeed(getCurrValue());

  auto *IfBodyBlock =
      BasicBlock::Create(CodeGen.Context, "if-body", CurrBlock->getParent());
  auto *IfEndBlock =
      BasicBlock::Create(CodeGen.Context, "if-end", CurrBlock->getParent());
  CodeGen.Builder->SetInsertPoint(IfBodyBlock);
  If->body()->accept(this);
  CodeGen.Builder->CreateBr(IfEndBlock);

  CodeGen.Builder->SetInsertPoint(CurrBlock);
  if (If->else_block()) {
    auto *IfElseBlock =
        BasicBlock::Create(CodeGen.Context, "if-else", CurrBlock->getParent());
    CodeGen.Builder->CreateCondBr(CondValue, IfBodyBlock, IfElseBlock);

    CodeGen.Builder->SetInsertPoint(IfElseBlock);
    If->else_block()->accept(this);
    CodeGen.Builder->CreateBr(IfEndBlock);
  } else {
    CodeGen.Builder->CreateCondBr(CondValue, IfBodyBlock, IfEndBlock);
  }
  CodeGen.Builder->SetInsertPoint(IfEndBlock);
}

void CodeGenVisitor::visit(ast::while_operator *While) {
  auto *CurrBlock = CodeGen.createBlockAndLinkWith(
      CodeGen.Builder->GetInsertBlock(), "while-cond");
  // Condition codegen
  While->condition()->accept(this);
  auto *EntryCondValue = CodeGen.createCondValueIfNeed(getCurrValue());

  auto *WhileBodyBlock =
      BasicBlock::Create(CodeGen.Context, "while-body", CurrBlock->getParent());
  auto *WhileEndBlock =
      BasicBlock::Create(CodeGen.Context, "while-end", CurrBlock->getParent());
  CodeGen.Builder->CreateCondBr(EntryCondValue, WhileBodyBlock, WhileEndBlock);

  CodeGen.Builder->SetInsertPoint(WhileBodyBlock);
  // While body codegen
  While->body()->accept(this);

  While->condition()->accept(this);
  auto *CondValue = CodeGen.createCondValueIfNeed(getCurrValue());
  CodeGen.Builder->CreateCondBr(CondValue, WhileBodyBlock, WhileEndBlock);
  CodeGen.Builder->SetInsertPoint(WhileEndBlock);
}

void CodeGenVisitor::visit(ast::read_expression * /*unused*/) {
  auto *ScanType = FunctionType::get(CodeGen.getInt32Ty(), false);
  auto *ScanFunc =
      CodeGen.Mod->getFunction(codegen::IRCodeGenerator::ParaCLScanFuncName);
  auto *ScanRetVal = CodeGen.Builder->CreateCall(ScanType, ScanFunc);
  setCurrValue(ScanRetVal);
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
