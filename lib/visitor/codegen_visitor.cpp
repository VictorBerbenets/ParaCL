#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/GlobalVariable.h>
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
  Builder().SetInsertPoint(EntryBlock);

  // Generating LLVM IR for main objects
  for (auto &&CurStatement : *RootBlock)
    CurStatement->accept(this);

  // Create exit block to end instruction insertion
  CodeGen.createBlockAndLinkWith(Builder().GetInsertBlock(), "pcl_exit");
  // Return void for __pcl_start
  Builder().CreateRetVoid();
}

void CodeGenVisitor::visit(ast::statement_block *StmBlock) {
  for (auto &&CurStatement : *StmBlock)
    CurStatement->accept(this);
}

void CodeGenVisitor::visit(ast::calc_expression *CalcExpr) {
  auto *Lhs = getValueAfterAccept(CalcExpr->left());
  auto *Rhs = getValueAfterAccept(CalcExpr->right());
  Lhs = Builder().CreateZExtOrTrunc(Lhs, CodeGen.getInt32Ty());
  Rhs = Builder().CreateZExtOrTrunc(Rhs, CodeGen.getInt32Ty());

  switch (CalcExpr->type()) {
  case ast::CalcOp::ADD:
    setCurrValue(Builder().CreateAdd(Lhs, Rhs));
    break;
  case ast::CalcOp::SUB:
    setCurrValue(Builder().CreateSub(Lhs, Rhs));
    break;
  case ast::CalcOp::MUL:
    setCurrValue(Builder().CreateMul(Lhs, Rhs));
    break;
  case ast::CalcOp::PERCENT:
    setCurrValue(Builder().CreateSRem(Lhs, Rhs));
    break;
  case ast::CalcOp::DIV:
    setCurrValue(Builder().CreateSDiv(Lhs, Rhs));
    break;
  default:
    llvm_unreachable("unrecognized type for calc_expression");
  }
}

void CodeGenVisitor::visit(ast::logic_expression *LogicExp) {
  auto LogTy = LogicExp->type();
  switch (LogTy) {
  case ast::LogicOp::AND:
    setCurrValue(createLogicAnd(LogicExp));
    break;
  case ast::LogicOp::OR:
    setCurrValue(createLogicOr(LogicExp));
    break;
  default:
    auto *Lhs = getValueAfterAccept(LogicExp->left());
    auto *Rhs = getValueAfterAccept(LogicExp->right());
    Lhs = Builder().CreateZExtOrTrunc(Lhs, CodeGen.getInt32Ty());
    Rhs = Builder().CreateZExtOrTrunc(Rhs, CodeGen.getInt32Ty());
    switch (LogTy) {
    case ast::LogicOp::LESS:
      setCurrValue(Builder().CreateICmpSLT(Lhs, Rhs));
      break;
    case ast::LogicOp::LESS_EQ:
      setCurrValue(Builder().CreateICmpSLE(Lhs, Rhs));
      break;
    case ast::LogicOp::GREATER:
      setCurrValue(Builder().CreateICmpSGT(Lhs, Rhs));
      break;
    case ast::LogicOp::GREATER_EQ:
      setCurrValue(Builder().CreateICmpSGE(Lhs, Rhs));
      break;
    case ast::LogicOp::EQ:
      setCurrValue(Builder().CreateICmpEQ(Lhs, Rhs));
      break;
    case ast::LogicOp::NEQ:
      setCurrValue(Builder().CreateICmpNE(Lhs, Rhs));
      break;
    default:
      llvm_unreachable("unrecognized type for logic expression");
    }
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
    setCurrValue(Builder().CreateNeg(Val));
    break;
  case ast::UnOp::NEGATE:
    setCurrValue(Builder().CreateICmpEQ(Val, CodeGen.createConstantInt32(0)));
    break;
  default:
    llvm_unreachable("unrecognized type for un_operator");
  }
}

void CodeGenVisitor::visit(ast::number *Num) {
  auto Val = CodeGen.createConstantInt32(Num->get_value(), true);
  setCurrValue(Val);
}

void CodeGenVisitor::visit(ast::variable *Var) {
  auto DeclKey = SymTbl.getDeclKeyFor(Var->entityKey());
  auto *Value = ValManager.getValueFor(DeclKey);
  assert(Value);
  auto *AllocTy = static_cast<AllocaInst *>(Value)->getAllocatedType();
  assert(AllocTy);
  auto *Load = Builder().CreateLoad(AllocTy, Value, Var->name());
  setCurrValue(Load);
}

void CodeGenVisitor::visit(ast::assignment *Assign) {
  auto *InitVal = getValueAfterAccept(Assign->getIdentExp());
  assert(InitVal);
  auto EntityKey = Assign->entityKey();
  if (!SymTbl.isDefined(EntityKey)) {
    auto InitValTy = InitVal->getType();
    if (InitValTy->isIntegerTy()) {
      SymTbl.tryDefine(EntityKey, InitValTy);
      auto *Alloca =
          Builder().CreateAlloca(CodeGen.getInt32Ty(), nullptr, Assign->name());

      Builder().CreateStore(InitVal, Alloca);
      ValManager.linkValueWithName(SymTbl.getDeclKeyFor(EntityKey), Alloca);
    } else {
      InitVal->setName(Assign->name());
      SymTbl.tryDefine(EntityKey, ValManager.getTypeFor(InitVal));
      ValManager.linkValueWithName(SymTbl.getDeclKeyFor(EntityKey), InitVal);
    }
  } else {
    Builder().CreateStore(
        InitVal, ValManager.getValueFor(SymTbl.getDeclKeyFor(EntityKey)));
  }
}

void CodeGenVisitor::visit(ast::if_operator *If) {
  // Evaluate condition
  auto *CondValue =
      CodeGen.createCondValueIfNeed(getValueAfterAccept(If->condition()));

  auto *CurrBlock = Builder().GetInsertBlock();
  auto *IfBodyBlock =
      BasicBlock::Create(CodeGen.Context, "if-body", CurrBlock->getParent());
  auto *IfEndBlock =
      BasicBlock::Create(CodeGen.Context, "if-end", CurrBlock->getParent());
  Builder().SetInsertPoint(IfBodyBlock);
  If->body()->accept(this);
  Builder().CreateBr(IfEndBlock);

  Builder().SetInsertPoint(CurrBlock);
  if (If->else_block()) {
    auto *IfElseBlock =
        BasicBlock::Create(CodeGen.Context, "if-else", CurrBlock->getParent());
    Builder().CreateCondBr(CondValue, IfBodyBlock, IfElseBlock);

    Builder().SetInsertPoint(IfElseBlock);
    If->else_block()->accept(this);
    Builder().CreateBr(IfEndBlock);
  } else {
    Builder().CreateCondBr(CondValue, IfBodyBlock, IfEndBlock);
  }
  Builder().SetInsertPoint(IfEndBlock);
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
  auto *ScanRetVal = Builder().CreateCall(ScanType, ScanFunc);
  setCurrValue(ScanRetVal);
}

void CodeGenVisitor::visit(ast::print_function *PrintFuncNode) {
  auto *PrintVal = getValueAfterAccept(PrintFuncNode->get());
  printIntegerValue(PrintVal);
}

void CodeGenVisitor::visit(ast::ArrayHolder *ArrStore) {
  CodeGen.createBlockAndLinkWith(Builder().GetInsertBlock(),
                                 "array_creation_block");
  auto *Arr = ArrStore->get();
  assert(Arr);
  // Obtain information about the array: initial values and size.
  Arr->accept(this);
  // Create a struct type for the array: a pointer type and types for sizes (if
  // the array is multidimensional).
  auto *DataTy = CodeGen.getInt32Ty();
  auto *DataPtrTy = PointerType::get(DataTy, 0);
  DataLayout Layout(&Module());
  unsigned ElementSize = Layout.getTypeAllocSize(CodeGen.getInt32Ty());

  SmallVector<Type *> Types;
  Types.reserve(ArrInfo.Sizes.size() + 1);
  Types.push_back(DataPtrTy);
  llvm::transform(ArrInfo.Sizes, std::back_inserter(Types),
                  [](auto *Sz) { return Sz->getType(); });

  auto *ArrStructTy = StructType::get(CodeGen.Context, Types);
  auto *AllocaStructPtr =
      Builder().CreateAlloca(ArrStructTy, nullptr, "array_struct");
  auto *ArrPtrGEP = Builder().CreateStructGEP(ArrStructTy, AllocaStructPtr, 0);
  // Fill in the fields of the structure that store the dimension of the array
  for (unsigned Index = 1; Index <= ArrInfo.Sizes.size(); ++Index) {
    auto *PtrGEP =
        Builder().CreateStructGEP(ArrStructTy, AllocaStructPtr, Index);
    Builder().CreateStore(ArrInfo.Sizes[Index - 1], PtrGEP);
  }

  ValManager.setValueTypeLink(AllocaStructPtr, ArrStructTy);
  setCurrValue(AllocaStructPtr);
  if (ArrInfo.isConstantSizes()) {
    // Use Alloca instruction because we know the size
    unsigned ArrSize = 1;
    // Calculate array size
    llvm::for_each(ArrInfo.Sizes, [&ArrSize](auto *Sz) {
      auto *ConstIntVal = dyn_cast<ConstantInt>(Sz);
      assert(ConstIntVal);
      ArrSize *= ConstIntVal->getZExtValue();
    });
    auto *ArrType = ArrayType::get(DataTy, ArrSize);
    auto *AllocaArr = Builder().CreateAlloca(ArrType, nullptr, "array");
    Builder().CreateStore(AllocaArr, ArrPtrGEP);

    if (ArrSize == 0)
      return;

    if (ArrInfo.isConstantData()) {
      SmallVector<Constant *> InitValues;
      InitValues.reserve(ArrSize);
      auto *TotalBytes = ConstantInt::get(DataTy, ArrSize * ElementSize);
      if (ArrInfo.Data.size() != 1) {
        auto InitData = ArrInfo.tryConvertDataToConstant();
        assert(InitData.has_value());
        InitValues = std::move(InitData.value());
      } else {
        auto *ConstVal = dyn_cast<ConstantInt>(ArrInfo.Data.front());
        assert(ConstVal);
        for (unsigned Id = 0; Id < ArrSize; ++Id) {
          InitValues.push_back(
              ConstantInt::get(DataTy, ConstVal->getSExtValue()));
        }
      }
      auto *ConstantInitializer = ConstantArray::get(ArrType, InitValues);
      auto *GlobalInitPtr =
          new GlobalVariable(*CodeGen.Mod.get(), ArrType, true,
                             Function::ExternalLinkage, ConstantInitializer);
      Builder().CreateMemCpy(AllocaArr, MaybeAlign(ElementSize), GlobalInitPtr,
                             MaybeAlign(ElementSize), TotalBytes);
    } else {
      if (ArrInfo.Data.size() == 1) {
        auto *InitValue = Builder().CreateLoad(DataTy, ArrInfo.Data.front());
        for (unsigned Id = 0; Id < ArrSize; ++Id) {
          auto *ArrayValueAlloc = Builder().CreateAlloca(DataTy);
          Builder().CreateStore(InitValue, ArrayValueAlloc);
          ArrInfo.Data.push_back(Builder().CreateLoad(DataTy, ArrayValueAlloc));
        }
      }
      ArrInfo.fillArrayWithData(Builder(), AllocaArr, DataTy);
    }
  } else {
    // Heap allocation
    Constant *ElemSize = ConstantInt::get(DataTy, ElementSize);
    ElemSize = ConstantExpr::getTruncOrBitCast(ElemSize, DataTy);

    Value *ArrSize = ConstantInt::get(DataTy, 1);
    llvm::for_each(ArrInfo.Sizes, [&](auto *Sz) {
      ArrSize = Builder().CreateMul(ArrSize, Sz);
    });
    auto *ArraySizeAlloca = Builder().CreateAlloca(DataTy);
    Builder().CreateStore(ArrSize, ArraySizeAlloca);
    auto *ArraySize = Builder().CreateLoad(DataTy, ArraySizeAlloca);
    auto *MallocCall = Builder().CreateMalloc(DataTy, 0, ElemSize, ArraySize);
    Builder().CreateStore(MallocCall, ArrPtrGEP);
    // Prepare data if need
    if (ArrInfo.Data.size() == 1) {
      auto *IndexAlloca = Builder().CreateAlloca(DataTy);
      Builder().CreateStore(ConstantInt::get(DataTy, 0), IndexAlloca);
      Value *IndexValue = Builder().CreateLoad(DataTy, IndexAlloca);

      auto *Cond = Builder().CreateICmpSLT(IndexValue, ArraySize);
      auto [BodyWhile, EndWhile] = createStartWhile(Cond);
      IndexValue = Builder().CreateLoad(DataTy, IndexAlloca);
      auto *GEPPtr = Builder().CreateGEP(DataTy, MallocCall, IndexValue);
      Builder().CreateStore(ArrInfo.Data.front(), GEPPtr);
      // Increment the loop index
      IndexValue = Builder().CreateAdd(IndexValue, ConstantInt::get(DataTy, 1));
      Builder().CreateStore(IndexValue, IndexAlloca);

      Cond = Builder().CreateICmpSLT(IndexValue, ArraySize);
      createEndWhile(Cond, BodyWhile, EndWhile);
    } else {
      ArrInfo.fillArrayWithData(Builder(), MallocCall, DataTy);
    }
  }

  ArrInfo.clear();
  CodeGen.createBlockAndLinkWith(Builder().GetInsertBlock());
}

void CodeGenVisitor::visit(ast::UniformArray *UnifArr) {
  auto *InitVal = getValueAfterAccept(UnifArr->getInitExpr());
  auto *Size = getValueAfterAccept(UnifArr->getSize());

  if (InitVal)
    ArrInfo.pushData(InitVal);
  ArrInfo.pushSize(Size);
  setCurrValue(nullptr);
}

void CodeGenVisitor::visit(ast::PresetArray *PresetArr) {
  llvm::for_each(*PresetArr, [&](auto *Exp) {
    auto *Val = getValueAfterAccept(Exp);
    if (Val)
      ArrInfo.pushData(Val);
  });

  ArrInfo.clearSize();
  ArrInfo.pushSize(CodeGen.createConstantInt32(ArrInfo.Data.size()));
  setCurrValue(nullptr);
}

void CodeGenVisitor::visit(ast::ArrayAccess *ArrAccess) {
  auto ElemPtr = getArrayAccessPtr(ArrAccess);
  auto *Load = Builder().CreateLoad(CodeGen.getInt32Ty(), ElemPtr);
  setCurrValue(Load);
}

void CodeGenVisitor::visit(ast::ArrayAccessAssignment *Assign) {
  auto *AccessPtr = getArrayAccessPtr(Assign->getArrayAccess());
  auto *IdentValue = getValueAfterAccept(Assign->getIdentExp());
  Builder().CreateStore(IdentValue, AccessPtr);
}

Value *CodeGenVisitor::getArrayAccessPtr(ast::ArrayAccess *ArrAccess) {
  auto *DataTy = CodeGen.getInt32Ty();
  llvm::SmallVector<llvm::Value *> Indexes;
  Indexes.reserve(ArrAccess->getSize() + 1);
  // Get access indexes for an array
  llvm::for_each(*ArrAccess, [&](auto *Exp) {
    auto *IndexVal = getValueAfterAccept(Exp);
    assert(IndexVal->getType()->isIntegerTy());
    Indexes.push_back(IndexVal);
  });

  auto DeclKey = SymTbl.getDeclKeyFor(ArrAccess->entityKey());
  auto *ArrStructPtr = ValManager.getValueFor(DeclKey);
  assert(ArrStructPtr);
  auto *ArrType = ValManager.getTypeFor(ArrStructPtr);
  assert(ArrType);
  assert(ArrType->isStructTy());
  Value *AccessIndex = ConstantInt::get(DataTy, 0);
  for (unsigned i = 0, IndexNum = Indexes.size(); i < IndexNum; ++i) {
    auto *CurrIndex = Indexes[i];
    Value *Multiplier = ConstantInt::get(DataTy, 1);
    for (unsigned j = i + 1; j < IndexNum; ++j) {
      auto *DimPtr = Builder().CreateStructGEP(ArrType, ArrStructPtr, j);
      auto *Dim = Builder().CreateLoad(DataTy, DimPtr);
      Multiplier = Builder().CreateMul(Multiplier, Dim);
    }
    Value *Term = Builder().CreateMul(CurrIndex, Multiplier);
    AccessIndex = Builder().CreateAdd(AccessIndex, Term, "access_index");
  }
  auto *GEPArrPtr = Builder().CreateStructGEP(ArrType, ArrStructPtr, 0);
  auto *ArrPtr = Builder().CreateLoad(PointerType::get(DataTy, 0), GEPArrPtr);
  return Builder().CreateGEP(DataTy, ArrPtr, AccessIndex);
}

std::pair<BasicBlock *, BasicBlock *>
CodeGenVisitor::createStartWhile(Value *Condition) {
  auto *CurrBlock =
      CodeGen.createBlockAndLinkWith(Builder().GetInsertBlock(), "while-cond");
  // Condition codegen
  auto *EntryCondValue = CodeGen.createCondValueIfNeed(Condition);

  auto *WhileBodyBlock =
      BasicBlock::Create(CodeGen.Context, "while-body", CurrBlock->getParent());
  auto *WhileEndBlock =
      BasicBlock::Create(CodeGen.Context, "while-end", CurrBlock->getParent());
  Builder().CreateCondBr(EntryCondValue, WhileBodyBlock, WhileEndBlock);

  Builder().SetInsertPoint(WhileBodyBlock);
  return std::make_pair(WhileBodyBlock, WhileEndBlock);
}

void CodeGenVisitor::createEndWhile(Value *Condition, BasicBlock *BodyBlock,
                                    BasicBlock *EndBlock) {
  auto *CondValue = CodeGen.createCondValueIfNeed(Condition);
  Builder().CreateCondBr(CondValue, BodyBlock, EndBlock);
  Builder().SetInsertPoint(EndBlock);
}

Value *CodeGenVisitor::createLogicAnd(ast::logic_expression *LogExp) {
  auto *DataTy = CodeGen.getInt32Ty();
  auto *Lhs = getValueAfterAccept(LogExp->left());
  Lhs = Builder().CreateZExt(Lhs, DataTy);

  auto *LhsBlock = Builder().GetInsertBlock();
  auto *IsLhsNonZero =
      Builder().CreateICmpNE(Lhs, ConstantInt::get(DataTy, 0), "tobool");
  auto *FalseBlock =
      BasicBlock::Create(CodeGen.Context, "and.false", LhsBlock->getParent());
  auto *RhsBlock = BasicBlock::Create(CodeGen.Context, "and.rhs",
                                      LhsBlock->getParent(), FalseBlock);
  auto *MergeBlock =
      BasicBlock::Create(CodeGen.Context, "and.end", LhsBlock->getParent());
  Builder().CreateCondBr(IsLhsNonZero, RhsBlock, FalseBlock);

  Builder().SetInsertPoint(RhsBlock);
  auto *Rhs = getValueAfterAccept(LogExp->right());
  Rhs = Builder().CreateZExt(Rhs, DataTy);
  auto *IsRhsNonZero =
      Builder().CreateICmpNE(Rhs, ConstantInt::get(DataTy, 0), "tobool");
  Builder().CreateBr(MergeBlock);

  Builder().SetInsertPoint(FalseBlock);
  Builder().CreateBr(MergeBlock);

  Builder().SetInsertPoint(MergeBlock);
  auto *Phi = Builder().CreatePHI(Type::getInt1Ty(CodeGen.Context), 2);
  Phi->addIncoming(IsRhsNonZero, RhsBlock);
  Phi->addIncoming(Builder().getFalse(), FalseBlock);

  return Phi;
}

Value *CodeGenVisitor::createLogicOr(ast::logic_expression *LogExp) {
  auto *DataTy = CodeGen.getInt32Ty();
  auto *Lhs = getValueAfterAccept(LogExp->left());
  Lhs = Builder().CreateZExt(Lhs, DataTy);

  auto *LhsBlock = Builder().GetInsertBlock();
  auto *IsLhsNonZero =
      Builder().CreateICmpNE(Lhs, ConstantInt::get(DataTy, 0), "tobool");
  auto *RhsBlock =
      BasicBlock::Create(CodeGen.Context, "or.rhs", LhsBlock->getParent());
  auto *MergeBlock =
      BasicBlock::Create(CodeGen.Context, "or.end", LhsBlock->getParent());
  Builder().CreateCondBr(IsLhsNonZero, MergeBlock, RhsBlock);

  Builder().SetInsertPoint(RhsBlock);
  auto *Rhs = getValueAfterAccept(LogExp->right());
  Rhs = Builder().CreateZExt(Rhs, DataTy);
  auto *IsRhsNonZero =
      Builder().CreateICmpNE(Rhs, ConstantInt::get(DataTy, 0), "tobool");
  Builder().CreateBr(MergeBlock);

  Builder().SetInsertPoint(MergeBlock);
  auto *Phi = Builder().CreatePHI(Type::getInt1Ty(CodeGen.Context), 2);
  Phi->addIncoming(Builder().getTrue(), LhsBlock);
  Phi->addIncoming(IsRhsNonZero, RhsBlock);

  return Phi;
}

void CodeGenVisitor::printIntegerValue(Value *Val) {
  assert(Val->getType()->isIntegerTy());

  SmallVector<Value *, 1> ArgValue{Val};
  SmallVector<Type *, 1> ArgTy{CodeGen.getInt32Ty()};
  auto *PrintType = FunctionType::get(CodeGen.getVoidTy(), ArgTy, false);
  auto *PrintFunc =
      CodeGen.Mod->getFunction(codegen::IRCodeGenerator::ParaCLPrintFuncName);
  Builder().CreateCall(PrintType, PrintFunc, ArgValue);
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
