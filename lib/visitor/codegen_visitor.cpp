#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/Support/FormatVariadic.h>

#include "ast_includes.hpp"
#include "codegen_visitor.hpp"

namespace paracl {

using ResultTy = CodeGenVisitor::ResultTy;

CodeGenVisitor::CodeGenVisitor(StringRef ModuleName) : CodeGen(ModuleName) {}

// This visit method for root basic block represents the main module of the
// program (global scope). The main function that launches the program is
// created here (__pcl_start)
ResultTy CodeGenVisitor::visit(ast::root_statement_block *RootBlock) {
  auto *PCLStartFunc = CodeGen.createFunction(
      CodeGen.getVoidTy(), Function::ExternalLinkage,
      codegen::IRCodeGenerator::ParaCLStartFuncName, false);
  // Create entry block to start instruction insertion
  auto *EntryBlock =
      BasicBlock::Create(CodeGen.Context, "pcl_entry", PCLStartFunc);
  Builder().SetInsertPoint(EntryBlock);

  // Generating LLVM IR for main objects
  for (auto &&CurStatement : *RootBlock)
    acceptASTNode(CurStatement);
  freeResources(RootBlock);

  // Create exit block to end instruction insertion
  CodeGen.createBlockAndLinkWith(Builder().GetInsertBlock(), "pcl_exit");
  // Return void for __pcl_start
  Builder().CreateRetVoid();
  return createWrapperRef();
}

ResultTy CodeGenVisitor::visit(ast::statement_block *StmBlock) {
  for (auto &&CurStatement : *StmBlock)
    acceptASTNode(CurStatement);
  freeResources(StmBlock);
  return createWrapperRef();
}

ResultTy CodeGenVisitor::visit(ast::calc_expression *CalcExpr) {
  auto &Lhs = acceptASTNode(CalcExpr->left());
  auto &Rhs = acceptASTNode(CalcExpr->right());
  assert(Lhs && Rhs);
  Lhs = Builder().CreateZExtOrTrunc(Lhs, CodeGen.getInt32Ty());
  Rhs = Builder().CreateZExtOrTrunc(Rhs, CodeGen.getInt32Ty());

  switch (CalcExpr->type()) {
  case ast::CalcOp::ADD:
    return createWrapperRef(Builder().CreateAdd(Lhs, Rhs));
    break;
  case ast::CalcOp::SUB:
    return createWrapperRef(Builder().CreateSub(Lhs, Rhs));
    break;
  case ast::CalcOp::MUL:
    return createWrapperRef(Builder().CreateMul(Lhs, Rhs));
    break;
  case ast::CalcOp::PERCENT:
    return createWrapperRef(Builder().CreateSRem(Lhs, Rhs));
    break;
  case ast::CalcOp::DIV:
    return createWrapperRef(Builder().CreateSDiv(Lhs, Rhs));
    break;
  default:
    llvm_unreachable("unrecognized type for calc_expression");
  }
}

ResultTy CodeGenVisitor::visit(ast::logic_expression *LogicExp) {
  auto LogTy = LogicExp->type();
  switch (LogTy) {
  case ast::LogicOp::AND:
    return createWrapperRef(createLogicAnd(LogicExp));
    break;
  case ast::LogicOp::OR:
    return createWrapperRef(createLogicOr(LogicExp));
    break;
  default:
    auto &Lhs = acceptASTNode(LogicExp->left());
    auto &Rhs = acceptASTNode(LogicExp->right());
    assert(Lhs && Rhs);
    Lhs = Builder().CreateZExtOrTrunc(Lhs, CodeGen.getInt32Ty());
    Rhs = Builder().CreateZExtOrTrunc(Rhs, CodeGen.getInt32Ty());
    switch (LogTy) {
    case ast::LogicOp::LESS:
      return createWrapperRef(Builder().CreateICmpSLT(Lhs, Rhs));
      break;
    case ast::LogicOp::LESS_EQ:
      return createWrapperRef(Builder().CreateICmpSLE(Lhs, Rhs));
      break;
    case ast::LogicOp::GREATER:
      return createWrapperRef(Builder().CreateICmpSGT(Lhs, Rhs));
      break;
    case ast::LogicOp::GREATER_EQ:
      return createWrapperRef(Builder().CreateICmpSGE(Lhs, Rhs));
      break;
    case ast::LogicOp::EQ:
      return createWrapperRef(Builder().CreateICmpEQ(Lhs, Rhs));
      break;
    case ast::LogicOp::NEQ:
      return createWrapperRef(Builder().CreateICmpNE(Lhs, Rhs));
      break;
    default:
      llvm_unreachable("unrecognized type for logic expression");
    }
  }
}

ResultTy CodeGenVisitor::visit(ast::un_operator *UnOper) {
  auto &Val = acceptASTNode(UnOper->arg());
  assert(Val);
  switch (UnOper->type()) {
  case ast::UnOp::PLUS:
    // Do nothing with the value
    return Val;
    break;
  case ast::UnOp::MINUS:
    return createWrapperRef(Builder().CreateNeg(Val));
    break;
  case ast::UnOp::NEGATE:
    return createWrapperRef(
        Builder().CreateICmpEQ(Val, CodeGen.createConstantInt32(0)));
    break;
  default:
    llvm_unreachable("unrecognized type for un_operator");
  }
}

ResultTy CodeGenVisitor::visit(ast::number *Num) {
  auto Val = CodeGen.createConstantInt32(Num->get_value(), true);
  return createWrapperRef(Val);
}

ResultTy CodeGenVisitor::visit(ast::variable *Var) {
  auto DeclKey = SymTbl.getDeclKeyFor(Var->entityKey());
  auto *Val = ValManager.getValueFor(DeclKey);
  assert(Val);
  if (auto *IsAllocaVal = dyn_cast<AllocaInst>(Val); IsAllocaVal) {
    auto AllocatedTy = IsAllocaVal->getAllocatedType();
    if (AllocatedTy->isIntegerTy())
      return createWrapperRef(
          Builder().CreateLoad(AllocatedTy, Val, Var->name()));
  }
  return createWrapperRef(Val);
}

ResultTy CodeGenVisitor::visit(ast::assignment *Assign) {
  auto &AcceptIdentVal = acceptASTNode(Assign->getIdentExp());
  assert(AcceptIdentVal);
  auto EntityKey = Assign->entityKey();
  Value *InitValue = AcceptIdentVal;
  if (!SymTbl.isDefined(EntityKey)) {
    SymTbl.tryDefine(EntityKey, ValManager.getTypeFor(AcceptIdentVal));
    if (InitValue->getType()->isIntegerTy()) {
      auto *Alloca =
          Builder().CreateAlloca(CodeGen.getInt32Ty(), 0, Assign->name());

      Builder().CreateStore(InitValue, Alloca);
      AcceptIdentVal = Alloca;
    }
    ValManager.linkValueWithName(SymTbl.getDeclKeyFor(EntityKey),
                                 AcceptIdentVal);
  } else {
    Builder().CreateStore(
        InitValue, ValManager.getValueFor(SymTbl.getDeclKeyFor(EntityKey)));
  }
  return createWrapperRef(InitValue);
}

ResultTy CodeGenVisitor::visit(ast::if_operator *If) {
  // Evaluate condition
  auto *CondValue =
      CodeGen.createCondValueIfNeed(acceptASTNode(If->condition()));

  auto *CurrBlock = Builder().GetInsertBlock();
  auto [IfBodyBlock, IfEndBlock] = createStartIf();
  acceptASTNode(If->body());
  Builder().CreateBr(IfEndBlock);

  Builder().SetInsertPoint(CurrBlock);
  if (If->else_block()) {
    auto *IfElseBlock =
        BasicBlock::Create(CodeGen.Context, "if.else", CurrBlock->getParent());
    Builder().CreateCondBr(CondValue, IfBodyBlock, IfElseBlock);

    Builder().SetInsertPoint(IfElseBlock);
    acceptASTNode(If->else_block());
    Builder().CreateBr(IfEndBlock);
  } else {
    Builder().CreateCondBr(CondValue, IfBodyBlock, IfEndBlock);
  }
  createEndIf(IfEndBlock);
  return createWrapperRef();
}

ResultTy CodeGenVisitor::visit(ast::while_operator *While) {
  auto [WhileBodyBlock, WhileEndBlock] =
      createStartWhile(acceptASTNode(While->condition()));
  // While body codegen
  acceptASTNode(While->body());

  createEndWhile(acceptASTNode(While->condition()), WhileBodyBlock,
                 WhileEndBlock);
  return createWrapperRef();
}

ResultTy CodeGenVisitor::visit(ast::read_expression * /*unused*/) {
  auto *ScanType = FunctionType::get(CodeGen.getInt32Ty(), false);
  auto *ScanFunc =
      CodeGen.Mod->getFunction(codegen::IRCodeGenerator::ParaCLScanFuncName);
  auto *ScanRetVal = Builder().CreateCall(ScanType, ScanFunc);
  return createWrapperRef(ScanRetVal);
}

ResultTy CodeGenVisitor::visit(ast::print_function *PrintFuncNode) {
  auto &PrintVal = acceptASTNode(PrintFuncNode->get());
  auto *PrintType = PrintVal->getType();
  assert(PrintType);

  if (PrintType->isPointerTy()) {
    assert(ArrInfoMap.contains(PrintVal));
    auto *ArrType = ValManager.getTypeFor(PrintVal);
    assert(ArrType);
    auto *DataTy = CodeGen.getInt32Ty();
    auto *ArrSize = ArrayInfo::calculateSize(Builder(), CodeGen.getInt32Ty(),
                                             ArrInfoMap[PrintVal].Sizes);
    // Print array in loop
    std::function<void(Value *)> LoopBody = [&](Value *LoopCounter) {
      auto *ElemPtr = Builder().CreateGEP(DataTy, PrintVal, LoopCounter);
      printIntegerValue(Builder().CreateLoad(DataTy, ElemPtr));
    };
    createUpCountLoop(ArrSize, LoopBody);
  } else if (PrintType->isIntegerTy())
    printIntegerValue(PrintVal);
  else {
    std::string S;
    llvm::raw_string_ostream OS(S);
    PrintType->print(OS);
    OS.flush();
    llvm_unreachable(
        llvm::formatv("incorrect print type: '{0}'", S).str().c_str());
  }
  return PrintVal;
}

ResultTy CodeGenVisitor::visit(ast::ArrayHolder *ArrStore) {
  CodeGen.createBlockAndLinkWith(Builder().GetInsertBlock(),
                                 "array.creat.block");
  auto *Arr = ArrStore->get();
  assert(Arr);
  // Obtain information about the array: initial values and size.
  acceptASTNode(Arr);

  auto *DataTy = CodeGen.getInt32Ty();
  Value *ArrPtr = createArray(DataTy, CurrArrInfo, ArrStore->scope());

  // Save the array's metadata (initialization values and dimensions). This may
  // be necessary when initializing a new array with values from an existing
  // one.
  [[maybe_unused]] auto [_, IsEmplaced] =
      ArrInfoMap.try_emplace(ArrPtr, std::move(CurrArrInfo));
  assert(IsEmplaced);
  // Clear the current array tracker.
  CurrArrInfo.clear();
  CodeGen.createBlockAndLinkWith(Builder().GetInsertBlock());

  ValManager.setValueTypeLink(ArrPtr, PointerType::get(DataTy, 0));
  return createWrapperRef(ArrPtr);
}

// In the next two functions, we recursively collecting information about an
// array: its size and the values of the elements. The array will be created in
// the visit ArrayHolder method of the class. The core idea is to gradually
// populate the ArrayInfo structure by traversing repeat (uniform) and array
// (preset) arrays. At the end of each traversal, we set the returned Value to
// nullptr (this signals that the next array’s initializer will be another
// array).
ResultTy CodeGenVisitor::visit(ast::UniformArray *UnifArr) {
  auto *DataTy = CodeGen.getInt32Ty();
  auto &InitVal = acceptASTNode(UnifArr->getInitExpr());
  auto &Size = acceptASTNode(UnifArr->getSize());
  assert(Size);
  auto *ConstSize = isConstantInt(Size);
  bool isConstantNotZeroSize = ConstSize && !ConstSize->isZero();
  auto Found = ArrInfoMap.find(InitVal);
  if (Found != ArrInfoMap.end())
    isConstantNotZeroSize &= isConstantData(Found->second.Sizes);

  auto TransformWithAlloca = [&](ArrayRef<Value *> From, auto &To) {
    llvm::transform(From, std::back_inserter(To), [&](auto *CopyVal) {
      return createLocalVariable(DataTy, CopyVal);
    });
  };

  if (isConstantNotZeroSize) {
    auto ArrSize = ConstSize->getZExtValue();
    if (InitVal) {
      CurrArrInfo.Data.reserve(ArrSize);
      if (auto *ConstVal = isConstantInt(InitVal); ConstVal) {
        std::generate_n(std::back_inserter(CurrArrInfo.Data), ArrSize,
                        [ConstVal] { return ConstVal; });
      } else if (Found != ArrInfoMap.end()) {
        // If the initializer is a previously created array, we retrieve its
        // metadata from the ArrayInfoMap. e.g
        //              ...
        // -- Arr = repeat(InitValue, 5);
        // -- Arr2 = repeat(Arr, 100);
        assert(Found != ArrInfoMap.end());
        auto &InitArrInfo = Found->second;
        // Copy Data
        if (isConstantData(InitArrInfo.Data))
          for (unsigned Id = 0; Id < ArrSize; ++Id)
            llvm::transform(InitArrInfo.Data,
                            std::back_inserter(CurrArrInfo.Data),
                            [](auto *CopyVal) { return CopyVal; });
        else
          for (unsigned Id = 0; Id < ArrSize; ++Id)
            TransformWithAlloca(InitArrInfo.Data, CurrArrInfo.Data);
        CurrArrInfo.Sizes.insert(CurrArrInfo.Sizes.begin(),
                                 InitArrInfo.Sizes.begin(),
                                 InitArrInfo.Sizes.end());
      } else {
        std::generate_n(std::back_inserter(CurrArrInfo.Data), ArrSize,
                        [&] { return createLocalVariable(DataTy, InitVal); });
      }
    } else {
      // If array is an initializer then copy its data.
      auto DataToFill = CurrArrInfo.Data;
      for (unsigned Id = 1; Id < ArrSize; ++Id)
        llvm::transform(DataToFill, std::back_inserter(CurrArrInfo.Data),
                        [&](auto *Val) -> Value * {
                          if (auto *ConstVal = dyn_cast<ConstantInt>(Val);
                              ConstVal)
                            return ConstVal;
                          return createLocalVariable(DataTy, Val);
                        });
    }
  } else if (InitVal) {
    if (Found != ArrInfoMap.end()) {
      // We have a previously created array as an initializer
      //              ...
      // -- Arr = repeat(InitValue, Sz);
      // -- Arr2 = repeat(Arr, Sz2);
      // Copy Data
      auto &InitArrInfo = Found->second;
      if (isConstantData(InitArrInfo.Data))
        llvm::transform(InitArrInfo.Data, std::back_inserter(CurrArrInfo.Data),
                        [](auto *CopyVal) { return CopyVal; });
      else
        TransformWithAlloca(InitArrInfo.Data, CurrArrInfo.Data);

      SmallVector<Value *> ExtraSizes;
      // Copy sizes
      if (isConstantData(InitArrInfo.Sizes))
        llvm::transform(InitArrInfo.Sizes, std::back_inserter(ExtraSizes),
                        [](auto *CopyVal) { return CopyVal; });
      else
        TransformWithAlloca(InitArrInfo.Sizes, ExtraSizes);

      CurrArrInfo.Sizes.insert(CurrArrInfo.Sizes.begin(), ExtraSizes.begin(),
                               ExtraSizes.end());
    } else {
      // Simple initializer, e.g
      //              ...
      // -- InitVal = 10;
      // -- Arr = repeat(InitVal, ArrSz);
      CurrArrInfo.pushData(InitVal);
    }
  }
  CurrArrInfo.pushSize(Size);
  return createWrapperRef(nullptr);
}

ResultTy CodeGenVisitor::visit(ast::PresetArray *PresetArr) {
  auto *DataTy = CodeGen.getInt32Ty();
  llvm::for_each(*PresetArr, [&](auto *Exp) {
    auto &Val = acceptASTNode(Exp);
    if (Val) {
      if (auto Found = ArrInfoMap.find(Val); Found != ArrInfoMap.end()) {
        // We have a previously created array as an initializer, e.g
        // -- Arr = repeat(10, 5);
        // -- Arr2 = array(..., Arr, ...);
        auto SizesOpt =
            tryConvertDataToConstant<ConstantInt>(Found->second.Sizes);
        assert(SizesOpt.has_value() &&
               "The preset array should be able to output the size");
        auto *InitArrSize = ArrayInfo::calculateSize(DataTy, SizesOpt.value());
        if (!InitArrSize->isZero()) {
          const auto &InitArrData = Found->second.Data;
          if (isConstantData(InitArrData))
            CurrArrInfo.pushData(InitArrData.begin(), InitArrData.end());
          else
            llvm::transform(InitArrData, std::back_inserter(CurrArrInfo.Data),
                            [&](auto *CopyVal) {
                              return createLocalVariable(DataTy, CopyVal);
                            });
        }

      } else {
        CurrArrInfo.pushData(Val);
      }
    }
  });

  CurrArrInfo.clearSize();
  CurrArrInfo.pushSize(CodeGen.createConstantInt32(CurrArrInfo.Data.size()));
  return createWrapperRef(nullptr);
}

ResultTy CodeGenVisitor::visit(ast::ArrayAccess *ArrAccess) {
  auto ElemPtr = getArrayAccessPtr(ArrAccess);
  auto *Load = Builder().CreateLoad(CodeGen.getInt32Ty(), ElemPtr);
  return createWrapperRef(Load);
}

ResultTy CodeGenVisitor::visit(ast::ArrayAccessAssignment *Assign) {
  auto *AccessPtr = getArrayAccessPtr(Assign->getArrayAccess());
  auto &IdentValue = acceptASTNode(Assign->getIdentExp());
  Builder().CreateStore(IdentValue, AccessPtr);
  return createWrapperRef(IdentValue);
}

Value *CodeGenVisitor::getArrayAccessPtr(ast::ArrayAccess *ArrAccess) {
  auto *DataTy = CodeGen.getInt32Ty();
  llvm::SmallVector<llvm::Value *> Indexes;
  Indexes.reserve(ArrAccess->getSize() + 1);
  // Get access indexes for an array
  llvm::for_each(*ArrAccess, [&](auto *Exp) {
    Value *IndexVal = acceptASTNode(Exp);
    assert(IndexVal->getType()->isIntegerTy());
    Indexes.push_back(IndexVal);
  });

  auto DeclKey = SymTbl.getDeclKeyFor(ArrAccess->entityKey());
  auto *ArrPtr = ValManager.getValueFor(DeclKey);
  assert(ArrPtr);
  [[maybe_unused]] auto *ArrType = ValManager.getTypeFor(ArrPtr);
  assert(ArrType);
  assert(ArrType->isPointerTy());

  // Calculate the offset of an element A[i1][i2]...[in]
  // in a n-dimensional array A of size D1 x D2 x ... x Dn:
  // offset = (i1 × D2 × D3 × ... × Dn +
  //     i2  D3  D4  ...  Dn +
  //     i3  D4  D5  ...  Dn +
  //      ...           +
  //     i(n-2)  D(n-1)  Dn   +
  //     i(n-1) * Dn       +
  //     in) * element_size
  Value *AccessIndex = ConstantInt::get(DataTy, 0);
  const auto &ArrSizes = ArrInfoMap[ArrPtr].Sizes;
  assert(ArrSizes.size() == Indexes.size());
  for (unsigned i = 0, IndexNum = Indexes.size(); i < IndexNum; ++i) {
    auto *CurrIndex = Indexes[i];
    Value *Multiplier = ConstantInt::get(DataTy, 1);
    for (unsigned j = i + 1; j < IndexNum; ++j) {
      auto *Dim = ArrSizes[IndexNum - j - 1];
      Multiplier = Builder().CreateMul(Multiplier, Dim);
    }
    Value *Term = Builder().CreateMul(CurrIndex, Multiplier);
    AccessIndex = Builder().CreateAdd(AccessIndex, Term, "access_index");
  }
  return Builder().CreateGEP(DataTy, ArrPtr, AccessIndex);
}

std::pair<BasicBlock *, BasicBlock *> CodeGenVisitor::createStartIf() {
  auto *CurrBlock = Builder().GetInsertBlock();
  auto *IfBodyBlock =
      BasicBlock::Create(CodeGen.Context, "if.body", CurrBlock->getParent());
  auto *IfEndBlock =
      BasicBlock::Create(CodeGen.Context, "if.end", CurrBlock->getParent());
  Builder().SetInsertPoint(IfBodyBlock);

  return {IfBodyBlock, IfEndBlock};
}

void CodeGenVisitor::createEndIf(BasicBlock *EndBlock) {
  Builder().SetInsertPoint(EndBlock);
}

std::pair<BasicBlock *, BasicBlock *>
CodeGenVisitor::createStartWhile(Value *Condition) {
  auto *CurrBlock =
      CodeGen.createBlockAndLinkWith(Builder().GetInsertBlock(), "while.cond");
  // Condition codegen
  auto *EntryCondValue = CodeGen.createCondValueIfNeed(Condition);

  auto *WhileBodyBlock =
      BasicBlock::Create(CodeGen.Context, "while.body", CurrBlock->getParent());
  auto *WhileEndBlock =
      BasicBlock::Create(CodeGen.Context, "while.end", CurrBlock->getParent());
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
  auto &Lhs = acceptASTNode(LogExp->left());
  assert(Lhs);
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
  auto &Rhs = acceptASTNode(LogExp->right());
  assert(Rhs);
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
  auto &Lhs = acceptASTNode(LogExp->left());
  assert(Lhs);
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
  auto &Rhs = acceptASTNode(LogExp->right());
  assert(Rhs);
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

Value *CodeGenVisitor::createArray(IntegerType *DataTy,
                                   const ArrayInfo &ArrInfo,
                                   ast::statement_block *CurrScope) {
  DataLayout Layout(&Module());
  unsigned ElementSize = Layout.getTypeAllocSize(DataTy);
  // Stack allocation
  if (auto ConstSizesOpt = tryConvertDataToConstant<ConstantInt>(ArrInfo.Sizes);
      ConstSizesOpt.has_value()) {
    auto *ArrSize = ArrayInfo::calculateSize(DataTy, ConstSizesOpt.value());
    return allocateLocalArray(DataTy, ArrInfo.Data, ArrSize->getZExtValue(),
                              ElementSize);
  }
  // Heap allocation
  Constant *ElemSize = ConstantInt::get(DataTy, ElementSize);
  // Calculate the array size to allocate memory.
  auto *ArraySize = ArrayInfo::calculateSize(Builder(), DataTy, ArrInfo.Sizes);
  auto *MallocCall = Builder().CreateMalloc(DataTy, 0, ElemSize, ArraySize);
  //  Create support array with init data
  auto *InitArrSize = ConstantInt::get(DataTy, ArrInfo.Data.size());
  auto *SupportArr = allocateLocalArray(
      DataTy, ArrInfo.Data, InitArrSize->getZExtValue(), ElementSize);

  std::function<void(Value *)> LoopBody = [&](Value *LoopCounter) {
    // Copy support array elements into the malloc-allocated array within the
    // loop.
    Value *InitVal;
    if (InitArrSize->equalsInt(1)) {
      InitVal = ArrInfo.Data.front();
    } else {
      // Access the indices of the support array by taking the remainder of
      // the main loop counter divided by the size of the support array. This
      // allows the malloc-allocated array to be filled in batches of values
      // from the support array.
      auto *InitArrInd = Builder().CreateSRem(LoopCounter, InitArrSize);
      auto *InitArrPtr = Builder().CreateGEP(DataTy, SupportArr, InitArrInd);
      InitVal = Builder().CreateLoad(DataTy, InitArrPtr);
    }

    auto *GEPtr = Builder().CreateGEP(DataTy, MallocCall, LoopCounter);
    Builder().CreateStore(InitVal, GEPtr);
  };
  // Initialize the malloc array in loop
  createUpCountLoop(ArraySize, LoopBody);

  // Dont't forget to free the pointer
  ResourcesToFree[CurrScope].push_back(MallocCall);
  return MallocCall;
}

AllocaInst *CodeGenVisitor::allocateLocalArray(Type *DataTy,
                                               ArrayRef<Value *> Elems,
                                               unsigned ArrSize,
                                               unsigned ElementSize) {
  // -- If the array is initialized with a mix of constants and patterns
  // (e.g., array(1, 2, repeat(1, 10), undef, array(-1, 1))), we use memcpy
  // to copy a precomputed array of constants into the allocated memory
  // region.
  // -- If the initialization values are not compile-time computable, we
  // fall back to a standard loop to fill the array element by element.
  // These scenarios use implementations based on memcpy or a loop,
  // depending on the case.
  // Note: zero-sized arrays (size 0) also produce an empty array.
  auto *ArrType = ArrayType::get(DataTy, ArrSize);
  auto *AllocaArr = Builder().CreateAlloca(ArrType, nullptr, "array");

  if (ArrSize == 0)
    // Don't initialize
    return AllocaArr;

  assert(ArrSize == Elems.size());
  if (auto ConstOptRange = tryConvertDataToConstant(Elems);
      ConstOptRange.has_value()) {
    auto *TotalBytes = ConstantInt::get(DataTy, ArrSize * ElementSize);
    auto *ConstantInitializer =
        ConstantArray::get(ArrType, ConstOptRange.value());
    auto *GlobalInitPtr =
        new GlobalVariable(*CodeGen.Mod.get(), ArrType, true,
                           Function::ExternalLinkage, ConstantInitializer);
    Builder().CreateMemCpy(AllocaArr, MaybeAlign(ElementSize), GlobalInitPtr,
                           MaybeAlign(ElementSize), TotalBytes);
  } else {
    fillArrayWithData(Builder(), AllocaArr, DataTy, Elems);
  }
  return AllocaArr;
}

LoadInst *CodeGenVisitor::createLocalVariable(Type *DataTy, Value *ToStore) {
  auto *Alloca = Builder().CreateAlloca(DataTy);
  Builder().CreateStore(ToStore, Alloca);
  return Builder().CreateLoad(DataTy, Alloca);
}

void CodeGenVisitor::fillArrayWithData(IRBuilder<> &Builder, Value *ArrPtr,
                                       Type *DataTy, ArrayRef<Value *> Data) {
  for (unsigned Id = 0; Id < Data.size(); ++Id) {
    auto *GEPPtr =
        Builder.CreateGEP(DataTy, ArrPtr, ConstantInt::get(DataTy, Id));
    Builder.CreateStore(Data[Id], GEPPtr);
  }
}

void CodeGenVisitor::freeResources(ast::statement_block *StmBlock) {
  if (ResourcesToFree.contains(StmBlock))
    llvm::for_each(ResourcesToFree[StmBlock],
                   [&](auto *ValToFree) { Builder().CreateFree(ValToFree); });
}

bool CodeGenVisitor::isConstantData(ArrayRef<Value *> Data) {
  return all_of(Data, [](auto *Val) { return isConstantInt(Val) != nullptr; });
}

ConstantInt *CodeGenVisitor::isConstantInt(Value *Val) {
  return dyn_cast<ConstantInt>(Val);
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
  acceptASTNode(RootBlock);
  printIRToOstream(Os);
}

void CodeGenVisitor::printIRToOstream(raw_ostream &Os) const {
  CodeGen.Mod->print(Os, nullptr);
}

} // namespace paracl
