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
  freeResources(RootBlock);

  // Create exit block to end instruction insertion
  CodeGen.createBlockAndLinkWith(Builder().GetInsertBlock(), "pcl_exit");
  // Return void for __pcl_start
  Builder().CreateRetVoid();
}

void CodeGenVisitor::visit(ast::statement_block *StmBlock) {
  for (auto &&CurStatement : *StmBlock)
    CurStatement->accept(this);
  freeResources(StmBlock);
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
  if (AllocTy->isIntegerTy())
    setCurrValue(Builder().CreateLoad(AllocTy, Value, Var->name()));
  else
    setCurrValue(Value);
}

void CodeGenVisitor::visit(ast::assignment *Assign) {
  auto *InitVal = getValueAfterAccept(Assign->getIdentExp());
  assert(InitVal);
  auto EntityKey = Assign->entityKey();
  if (!SymTbl.isDefined(EntityKey)) {
    SymTbl.tryDefine(EntityKey, ValManager.getTypeFor(InitVal));
    if (InitVal->getType()->isIntegerTy()) {
      auto *Alloca =
          Builder().CreateAlloca(CodeGen.getInt32Ty(), 0, Assign->name());

      Builder().CreateStore(InitVal, Alloca);
      InitVal = Alloca;
    }
    ValManager.linkValueWithName(SymTbl.getDeclKeyFor(EntityKey), InitVal);
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
  auto [IfBodyBlock, IfEndBlock] = createStartIf();
  If->body()->accept(this);
  Builder().CreateBr(IfEndBlock);

  Builder().SetInsertPoint(CurrBlock);
  if (If->else_block()) {
    auto *IfElseBlock =
        BasicBlock::Create(CodeGen.Context, "if.else", CurrBlock->getParent());
    Builder().CreateCondBr(CondValue, IfBodyBlock, IfElseBlock);

    Builder().SetInsertPoint(IfElseBlock);
    If->else_block()->accept(this);
    Builder().CreateBr(IfEndBlock);
  } else {
    Builder().CreateCondBr(CondValue, IfBodyBlock, IfEndBlock);
  }
  createEndIf(IfEndBlock);
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
                                 "array.creat.block");
  auto *Arr = ArrStore->get();
  assert(Arr);
  // Obtain information about the array: initial values and size.
  Arr->accept(this);

  auto *DataTy = CodeGen.getInt32Ty();
  DataLayout Layout(&Module());
  unsigned ElementSize = Layout.getTypeAllocSize(DataTy);

  // Create a struct type for the array: a pointer type and types for sizes (if
  // the array is multidimensional).
  SmallVector<Type *> Types;
  Types.reserve(CurrArrInfo.Sizes.size() + 1);
  Types.push_back(PointerType::get(DataTy, 0));
  llvm::transform(CurrArrInfo.Sizes, std::back_inserter(Types),
                  [](auto *Sz) { return Sz->getType(); });

  auto *ArrStructTy = StructType::get(CodeGen.Context, Types);
  auto *AllocaStructPtr =
      Builder().CreateAlloca(ArrStructTy, nullptr, "array.struct");
  auto *ArrPtrGEP = Builder().CreateStructGEP(ArrStructTy, AllocaStructPtr, 0);
  // Initialize the dimension size fields in the structure
  for (unsigned Index = 1; Index <= CurrArrInfo.Sizes.size(); ++Index) {
    auto *PtrGEP =
        Builder().CreateStructGEP(ArrStructTy, AllocaStructPtr, Index);
    Builder().CreateStore(CurrArrInfo.Sizes[Index - 1], PtrGEP);
  }

  ValManager.setValueTypeLink(AllocaStructPtr, ArrStructTy);
  setCurrValue(AllocaStructPtr);
  if (auto ConstSizesOpt =
          tryConvertDataToConstant<ConstantInt>(CurrArrInfo.Sizes);
      ConstSizesOpt.has_value()) {
    auto &ConstSizes = ConstSizesOpt.value();
    // Calculate array size
    auto *ArrSize = ArrayInfo::calculateSize(DataTy, ConstSizes);
    // -- If the array is initialized with a mix of constants and patterns
    // (e.g., array(1, 2, repeat(1, 10), undef, array(-1, 1))), we use memcpy
    // to copy a precomputed array of constants into the allocated memory
    // region.
    // -- If the initialization values are not compile-time computable, we
    // fall back to a standard loop to fill the array element by element.
    // These scenarios use implementations based on memcpy or a loop,
    // depending on the case.
    // Note: zero-sized arrays (size 0) also produce an empty array.
    auto *AllocaArr = createArrayWithData(DataTy, CurrArrInfo.Data,
                                          ArrSize->getZExtValue(), ElementSize);
    assert(AllocaArr);
    Builder().CreateStore(AllocaArr, ArrPtrGEP);
  } else {
    // Heap allocation
    Constant *ElemSize = ConstantInt::get(DataTy, ElementSize);

    // Calculate the array size to allocate memory.
    Value *ArrSize = ConstantInt::get(DataTy, 1);
    llvm::for_each(CurrArrInfo.Sizes, [&](auto *Sz) {
      ArrSize = Builder().CreateMul(ArrSize, Sz);
    });
    auto *ArraySize = createLocalVariable(DataTy, ArrSize);
    auto *MallocCall = Builder().CreateMalloc(DataTy, 0, ElemSize, ArraySize);
    Builder().CreateStore(MallocCall, ArrPtrGEP);
    // Create a loop and a counter to initialize the array.
    Value *IndexValue =
        createLocalVariable(DataTy, ConstantInt::get(DataTy, 0));
    auto *AllocaIndex = dyn_cast<LoadInst>(IndexValue)->getPointerOperand();

    auto *Cond = Builder().CreateICmpSLT(IndexValue, ArraySize);
    auto [BodyWhile, EndWhile] = createStartWhile(Cond);

    IndexValue = Builder().CreateLoad(DataTy, AllocaIndex);
    if (CurrArrInfo.Data.size() == 1) {
      // The array is initialized with a single value (e.g., a repeat array).
      // In this case, insert instructions in the loop body to access and write
      // the value into the array element.
      auto *GEPPtr = Builder().CreateGEP(DataTy, MallocCall, IndexValue);
      Builder().CreateStore(CurrArrInfo.Data.front(), GEPPtr);
    } else {
      // The array is initialized with a set of values. Here, create a support
      // array containing these values and copy its elements into the
      // malloc-allocated array within the loop. Access the indices of the
      // support array by taking the remainder of the main loop counter divided
      // by the size of the support array. This allows the malloc-allocated
      // array to be filled in batches of values from the support array.
      auto *InitArrSize = ConstantInt::get(DataTy, CurrArrInfo.Data.size());
      auto *AllocaArr = createArrayWithData(
          DataTy, CurrArrInfo.Data, InitArrSize->getZExtValue(), ElementSize);

      auto *InitArrInd = Builder().CreateSRem(IndexValue, InitArrSize);
      auto *InitArrPtr = Builder().CreateGEP(DataTy, AllocaArr, InitArrInd);
      auto *InitVal = Builder().CreateLoad(DataTy, InitArrPtr);

      auto *MallocPtr = Builder().CreateGEP(DataTy, MallocCall, IndexValue);
      Builder().CreateStore(InitVal, MallocPtr);
    }

    // Increment the loop index
    IndexValue = Builder().CreateAdd(IndexValue, ConstantInt::get(DataTy, 1));
    Builder().CreateStore(IndexValue, AllocaIndex);
    Cond = Builder().CreateICmpSLT(IndexValue, ArraySize);
    createEndWhile(Cond, BodyWhile, EndWhile);
    // Dont't forget to free the pointer
    ResourcesToFree[ArrStore->scope()].push_back(MallocCall);
  }

  // Save the array's metadata (initialization values and dimensions). This may
  // be necessary when initializing a new array with values from an existing
  // one.
  [[maybe_unused]] auto [_, IsEmplaced] =
      ArrInfoMap.try_emplace(AllocaStructPtr, std::move(CurrArrInfo));
  assert(IsEmplaced);
  // Clear the current array tracker.
  CurrArrInfo.clear();
  CodeGen.createBlockAndLinkWith(Builder().GetInsertBlock());
}

// In the next two functions, we recursively construct the array, which will
// ultimately be returned to the visit method of the ArrayHolder class. The core
// idea is to gradually populate the ArrayInfo structure by traversing repeat
// (uniform) and array (preset) arrays. At the end of each traversal, we set the
// returned Value to nullptr (this signals that the next array’s initializer
// will be another array).
void CodeGenVisitor::visit(ast::UniformArray *UnifArr) {
  auto *DataTy = CodeGen.getInt32Ty();
  auto *InitVal = getValueAfterAccept(UnifArr->getInitExpr());
  auto *Size = getValueAfterAccept(UnifArr->getSize());
  assert(Size);
  auto *ConstSize = isConstantInt(Size);
  bool isConstantNZeroSize = ConstSize && !ConstSize->isZero();
  auto Found = ArrInfoMap.find(InitVal);
  if (Found != ArrInfoMap.end())
    isConstantNZeroSize &= isConstantData(Found->second.Sizes);

  auto TransformWithAlloca = [&](ArrayRef<Value *> From, auto &To) {
    llvm::transform(From, std::back_inserter(To), [&](auto *CopyVal) {
      return createLocalVariable(DataTy, CopyVal);
    });
  };

  if (isConstantNZeroSize) {
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
  setCurrValue(nullptr);
}

void CodeGenVisitor::visit(ast::PresetArray *PresetArr) {
  auto *DataTy = CodeGen.getInt32Ty();
  llvm::for_each(*PresetArr, [&](auto *Exp) {
    auto *Val = getValueAfterAccept(Exp);
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

AllocaInst *CodeGenVisitor::createArrayWithData(Type *DataTy,
                                                ArrayRef<Value *> Elems,
                                                unsigned ArrSize,
                                                unsigned ElementSize) {
  auto *ArrType = ArrayType::get(DataTy, ArrSize);
  auto *AllocaArr = Builder().CreateAlloca(ArrType, nullptr, "array");

  if (ArrSize == 0)
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
