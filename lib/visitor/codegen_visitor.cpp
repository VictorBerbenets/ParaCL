#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/Support/FormatVariadic.h>

#include "ast_includes.hpp"
#include "codegen_visitor.hpp"

namespace paracl {

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
  auto *Lhs = getValueAfterAccept(CalcExpr->left());
  auto *Rhs = getValueAfterAccept(CalcExpr->right());

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

void CodeGenVisitor::visit(ast::logic_expression *LogicExp) {
  auto *Lhs = getValueAfterAccept(LogicExp->left());
  auto *Rhs = getValueAfterAccept(LogicExp->right());

  switch (LogicExp->type()) {
  case ast::LogicOp::AND:
    setCurrValue(CodeGen.Builder->CreateAnd(Lhs, Rhs));
    break;
  case ast::LogicOp::OR:
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
  auto *Val = getValueAfterAccept(UnOper->arg());

  switch (UnOper->type()) {
  case ast::UnOp::PLUS:
    // Do nothing with the value
    setCurrValue(Val);
    break;
  case ast::UnOp::MINUS:
    setCurrValue(CodeGen.Builder->CreateNeg(Val));
    break;
  case ast::UnOp::NEGATE:
    setCurrValue(
        CodeGen.Builder->CreateICmpEQ(Val, CodeGen.createConstantInt32(0)));
    break;
  default:
    llvm_unreachable("unrecognized type for un_operator");
  }
}

void CodeGenVisitor::visit(ast::number *Num) {
  auto Val = CodeGen.createConstantInt32(Num->get_value());
  ValueToType[Val] = CodeGen.getInt32Ty();
  setCurrValue(Val);
}

void CodeGenVisitor::visit(ast::variable *Var) {
  llvm::outs() << "VAR NAME = " << Var->name() << '\n';
  auto *Value = getValueForVar(Var->name());
  auto *AllocTy = static_cast<AllocaInst *>(Value)->getAllocatedType();
  auto *Load = CodeGen.Builder->CreateLoad(AllocTy, Value, Var->name());
  ValueToType[Load] = AllocTy;
  setCurrValue(Load);
}

void CodeGenVisitor::visit(ast::assignment *Assign) {
  llvm::outs() << "In assignment\n";
  auto *InitVal = getValueAfterAccept(Assign->getIdentExp());
  assert(InitVal);
  llvm::outs() << "BEFORE CONTAINS CHECK TYPE:\n";
  InitVal->getType()->print(llvm::outs());
  if (!NameToValue.contains(Assign->name())) {
    auto InitValTy = InitVal->getType();
    llvm::outs() << "TYPE:\n";
    InitValTy->print(llvm::outs());
    llvm::outs() << '\n';
    if (InitValTy->isIntegerTy()) {
      auto *Alloca = CodeGen.Builder->CreateAlloca(CodeGen.getInt32Ty(),
                                                   nullptr, Assign->name());
      CodeGen.Builder->CreateStore(InitVal, Alloca);
      NameToValue[Assign->name()] = Alloca;
    } else {
      InitVal->setName(Assign->name());
      NameToValue[Assign->name()] = InitVal;
    }
  } else {
    CodeGen.Builder->CreateStore(InitVal, NameToValue[Assign->name()]);
  }
}

void CodeGenVisitor::visit(ast::if_operator *If) {
  auto *CurrBlock = CodeGen.createBlockAndLinkWith(
      CodeGen.Builder->GetInsertBlock(), "if-cond");
  // Evaluate condition
  auto *CondValue =
      CodeGen.createCondValueIfNeed(getValueAfterAccept(If->condition()));

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
  auto [WhileBodyBlock, WhileEndBlock] =
      createStartWhile(getValueAfterAccept(While->condition()));
  // While body codegen
  While->body()->accept(this);

  createEndWhile(getValueAfterAccept(While->condition()), WhileBodyBlock,
                 WhileEndBlock);
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

  SmallVector<Value *, 1> ArgValue{getCurrValue()};
  SmallVector<Type *, 1> ArgTy{CodeGen.getInt32Ty()};
  auto *PrintType = FunctionType::get(CodeGen.getVoidTy(), ArgTy, false);
  auto *PrintFunc =
      CodeGen.Mod->getFunction(codegen::IRCodeGenerator::ParaCLPrintFuncName);
  CodeGen.Builder->CreateCall(PrintType, PrintFunc, ArgValue);
}

void CodeGenVisitor::visit(ast::ArrayHolder *ArrStore) {
  auto *CurrBlock = CodeGen.createBlockAndLinkWith(
      CodeGen.Builder->GetInsertBlock(), "array_creation_block");

  auto *Arr = ArrStore->get();
  assert(Arr);
  // Obtain information about the array: initial values and size.
  Arr->accept(this);
  // Create a struct type for the array: a pointer type and types for sizes (if
  // the array is multidimensional).
  auto *DataTy = CodeGen.getInt32Ty();
  auto *DataPtrTy = PointerType::get(DataTy, 0);
  SmallVector<Type *> Types;
  Types.reserve(ArrInfo.Sizes.size() + 1);
  Types.push_back(DataPtrTy);
  llvm::outs() << "TYPES SZ = " << Types.size() << '\n';
  llvm::outs() << "ARR INFO SZ = " << ArrInfo.Sizes.size() << '\n';
  llvm::transform(ArrInfo.Sizes, std::back_inserter(Types),
                  [](auto *Sz) { return Sz->getType(); });
  llvm::outs() << "TYPES SZ = " << Types.size() << '\n';

  auto *ArrStructTy = StructType::get(CodeGen.Context, Types);
  if (ArrInfo.isConstant()) {
    // Stack allocation
    unsigned ArrSize = 1;
    llvm::for_each(ArrInfo.Sizes, [&ArrSize](auto *Sz) {
      auto *ConstIntVal = dyn_cast<ConstantInt>(Sz);
      assert(ConstIntVal);
      ArrSize *= ConstIntVal->getZExtValue();
    });
    auto *AllocaStructPtr =
        CodeGen.Builder->CreateAlloca(ArrStructTy, nullptr, "arr-struct");
    auto *ArrPtrGEP =
        CodeGen.Builder->CreateStructGEP(ArrStructTy, AllocaStructPtr, 0);
    auto *ArrType = ArrayType::get(DataTy, ArrSize);
    auto *AllocaArr = CodeGen.Builder->CreateAlloca(ArrType, nullptr, "array");
    CodeGen.Builder->CreateStore(AllocaArr, ArrPtrGEP);
    ValueToType[ArrPtrGEP] = ArrType;
    llvm::outs() << "HOLDER PTR = " << ArrPtrGEP << '\n';
    for (unsigned Index = 1; Index <= ArrInfo.Sizes.size(); ++Index) {
      auto *PtrGEP =
          CodeGen.Builder->CreateStructGEP(ArrStructTy, AllocaStructPtr, Index);
      CodeGen.Builder->CreateStore(ArrInfo.Sizes[Index - 1], PtrGEP);
    }

    ValueToType[AllocaStructPtr] = ArrStructTy;
    setCurrValue(AllocaStructPtr);
  } else {
    // Heap allocation
  }
  ArrInfo.clear();

  CodeGen.createBlockAndLinkWith(CodeGen.Builder->GetInsertBlock());
}

void CodeGenVisitor::visit(ast::UniformArray *UnifArr) {
  auto *InitVal = getValueAfterAccept(UnifArr->getInitExpr());
  auto *Size = getValueAfterAccept(UnifArr->getSize());

  if (InitVal)
    ArrInfo.pushData(InitVal);
  llvm::outs() << "PUSH SIZE = " << Size << '\n';
  ArrInfo.pushSize(Size);
  setCurrValue(nullptr);
#if 0
  assert(InitVal);
  assert(Size);
  assert(Size->getType()->isIntegerTy()); 
  auto *ElementType = getValueType(InitVal);
  if (auto *ConstIntVal = isCompileTimeConstant<llvm::ConstantInt>(Size); ConstIntVal) {
    auto ArrSize = ConstIntVal->getZExtValue();
    llvm::outs() << "ArrSize = " << ArrSize << '\n';
    // Stack allocation
    auto *ArrType = llvm::ArrayType::get(ElementType, ArrSize);
    auto *ArrAloca = CodeGen.Builder->CreateAlloca(ArrType, 0, "uniform_arr");
    if (ElementType->isArrayTy())
      InitVal = CodeGen.Builder->CreateLoad(ElementType, InitVal);
    for (unsigned Index = 0; Index < ArrSize; ++Index) {
      auto *IndexVal = CodeGen.createConstantSInt32(Index);
      auto *ElemPtr = CodeGen.Builder->CreateInBoundsGEP(ArrType, ArrAloca, IndexVal, llvm::formatv("arr_elem{0}", Index));
      CodeGen.Builder->CreateStore(InitVal, ElemPtr);
    }
    auto *ArrPtrTy = llvm::PointerType::get(ArrType, 0);
    auto *ArrPtr = CodeGen.Builder->CreatePointerCast(ArrAloca, ArrPtrTy);
    ValueToType[ArrPtr] = ArrType;
    setCurrValue(ArrPtr);
  } else {
    // Heap allocation
    auto *Int32Ty = CodeGen.getInt32Ty();
    auto *AllocaSize = llvm::ConstantExpr::getSizeOf(ElementType);
    AllocaSize = llvm::ConstantExpr::getTruncOrBitCast(AllocaSize, Int32Ty);
    auto *MallocPtr = CodeGen.Builder->CreateMalloc(Int32Ty, ElementType, AllocaSize, Size); 
    auto *ArrPtr = CodeGen.Builder->CreateBitCast(MallocPtr, ElementType->getPointerTo());

    setCurrValue(ArrPtr);
  }
#endif
}

void CodeGenVisitor::visit(ast::PresetArray *PresetArr) {
  llvm::for_each(*PresetArr, [&](auto *Exp) {
    auto *Val = getValueAfterAccept(Exp);
    if (Val)
      ArrInfo.pushData(Val);
  });

  ArrInfo.clearSize();
  ArrInfo.pushSize(CodeGen.createConstantInt32(ArrInfo.Data.size(), false));
  setCurrValue(nullptr);
}

void CodeGenVisitor::visit(ast::ArrayAccess *ArrAccess) {
  llvm::SmallVector<llvm::Value *> Indexes;
  Indexes.reserve(ArrAccess->getSize() + 1);
  llvm::for_each(*ArrAccess, [&](auto *Exp) {
    auto *IndexVal = getValueAfterAccept(Exp);
    assert(IndexVal->getType()->isIntegerTy());
    Indexes.push_back(IndexVal);
  });

  auto *ArrStructPtr = NameToValue[ArrAccess->name()];
  assert(ValueToType[ArrStructPtr]->isStructTy());

  Value *AccessIndex = ConstantInt::get(Type::getInt64Ty(CodeGen.Context), 0);
  for (unsigned i = 0, IndexNum = Indexes.size(); i < IndexNum; ++i) {
    auto *CurrIndex = Indexes[i];
    Value *Multiplier = ConstantInt::get(CodeGen.getInt32Ty(), 1);
    for (unsigned j = i + 1; j < IndexNum; ++j) {
      auto *DimPtr = CodeGen.Builder->CreateStructGEP(ValueToType[ArrStructPtr],
                                                      ArrStructPtr, j);
      auto *Dim = CodeGen.Builder->CreateLoad(CodeGen.getInt32Ty(), DimPtr);
      llvm::outs() << "MULLLLLLL\n";
      Multiplier = CodeGen.Builder->CreateMul(Multiplier, Dim);
      llvm::outs() << "MULLLLLLL\n";
    }
    llvm::outs() << "LINE = " << __LINE__ << "\n";
    Value *Term = CodeGen.Builder->CreateMul(CurrIndex, Multiplier);
    AccessIndex = CodeGen.Builder->CreateAdd(AccessIndex, Term, "access_index");
  }
  auto *ArrPtr = CodeGen.Builder->CreateStructGEP(ValueToType[ArrStructPtr],
                                                  ArrStructPtr, 0);
  llvm::outs() << "Access PTR = " << ArrPtr << '\n';
  auto *ArrType = ArrayType::get(CodeGen.getInt32Ty(), 5);
  auto *ElemPtr = CodeGen.Builder->CreateGEP(
      ArrType, ArrPtr, {CodeGen.createConstantInt32(0), AccessIndex});
  auto *Load = CodeGen.Builder->CreateLoad(CodeGen.getInt32Ty(), ElemPtr);
  setCurrValue(Load);
  auto *Alloca = CodeGen.Builder->CreateAlloca(CodeGen.getInt32Ty(), nullptr);
  CodeGen.Builder->CreateStore(AccessIndex, Alloca);
  auto *Val = CodeGen.Builder->CreateLoad(CodeGen.getInt32Ty(), Alloca);
  SmallVector<Value *, 1> ArgValue{Val};
  SmallVector<Type *, 1> ArgTy{CodeGen.getInt32Ty()};
  auto *PrintType = FunctionType::get(CodeGen.getVoidTy(), ArgTy, false);
  auto *PrintFunc =
      CodeGen.Mod->getFunction(codegen::IRCodeGenerator::ParaCLPrintFuncName);
  CodeGen.Builder->CreateCall(PrintType, PrintFunc, ArgValue);
#if 0
  assert(ArrType);
  assert(ArrType->isArrayTy());
#endif
#if 0
  assert(ArrPtr);
  auto *GEPPtr =
      CodeGen.Builder->CreateStructGEP(ValueToType[ArrayPtr], ArrayPtr, );
  auto *Load = CodeGen.Builder->CreateLoad(CodeGen.getInt32Ty(), GEPPtr);
  setCurrValue(Load);
#endif
}

void CodeGenVisitor::visit(ast::ArrayAccessAssignment *Arr) {}

Value *CodeGenVisitor::getValueForVar(StringRef ValueName) {
  assert(NameToValue.contains(ValueName));
  return NameToValue[ValueName];
}

Type *CodeGenVisitor::getValueType(llvm::Value *Val) {
  assert(ValueToType.contains(Val));
  return ValueToType[Val];
}

std::pair<BasicBlock *, BasicBlock *>
CodeGenVisitor::createStartWhile(Value *Condition) {
  auto *CurrBlock = CodeGen.createBlockAndLinkWith(
      CodeGen.Builder->GetInsertBlock(), "while-cond");
  // Condition codegen
  auto *EntryCondValue = CodeGen.createCondValueIfNeed(Condition);

  auto *WhileBodyBlock =
      BasicBlock::Create(CodeGen.Context, "while-body", CurrBlock->getParent());
  auto *WhileEndBlock =
      BasicBlock::Create(CodeGen.Context, "while-end", CurrBlock->getParent());
  CodeGen.Builder->CreateCondBr(EntryCondValue, WhileBodyBlock, WhileEndBlock);

  CodeGen.Builder->SetInsertPoint(WhileBodyBlock);
  return std::make_pair(WhileBodyBlock, WhileEndBlock);
}

void CodeGenVisitor::createEndWhile(Value *Condition, BasicBlock *BodyBlock,
                                    BasicBlock *EndBlock) {
  auto *CondValue = CodeGen.createCondValueIfNeed(Condition);
  CodeGen.Builder->CreateCondBr(CondValue, BodyBlock, EndBlock);
  CodeGen.Builder->SetInsertPoint(EndBlock);
}

void CodeGenVisitor::generateIRCode(ast::root_statement_block *RootBlock,
                                    raw_ostream &Os) {
  RootBlock->accept(this);
  printIRToOstream(Os);
}

void CodeGenVisitor::printIRToOstream(raw_ostream &Os) const {
  CodeGen.Mod->print(Os, nullptr);
}

llvm::Value *CodeGenVisitor::getValueAfterAccept(ast::statement *Stm) {
  assert(Stm);
  Stm->accept(this);
  return CurrVal;
}

} // namespace paracl
