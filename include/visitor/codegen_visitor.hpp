#pragma once

#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Casting.h>

#include <functional>

#include "codegen.hpp"
#include "codegen_support.hpp"
#include "semantic_context.hpp"
#include "utils.hpp"
#include "visitor.hpp"

namespace paracl {
namespace codegen {

using namespace llvm;

class CodeGenWrapper: public ValueWrapper {
protected:
  enum class EntityType {LLVMValue, ArrayInfo};

  CodeGenWrapper(EntityType EntTy): EntityTy(EntTy) {}

public:
  bool isLLVMValueWrapper() const noexcept { return EntityTy == EntityType::LLVMValue; }
  bool isArrayInfoWrapper() const noexcept { return EntityTy == EntityType::ArrayInfo; }

private:
  EntityType EntityTy;
};

// Wrapper class for LLVM Values
class LLVMValueWrapper final: public CodeGenWrapper,
                         public ValueWrapperInterface<Value> {
public:
  LLVMValueWrapper(Value *Val): CodeGenWrapper(EntityType::LLVMValue), WrapperInterfaceTy(Val) {}
};

class ArrayInfoWrapper final: public CodeGenWrapper,
                        public ValueWrapperInterface<ArrayInfo> {
public:
  ArrayInfoWrapper(ArrayInfo *Val): CodeGenWrapper(EntityType::ArrayInfo), WrapperInterfaceTy(Val) {}
};

class CodeGenVisitor : public VisitorBase {

public:
  using DefaultValue = CodeGenWrapper;
  using DefaultResultTy = DefaultValue &;
  using CodeGenValue = LLVMValueWrapper;
  using ResultTy = CodeGenValue &;
  using ArrayInfoValue = ArrayInfoWrapper;
  using ResultArrayTy = ArrayInfoValue &;

  CodeGenVisitor(StringRef ModuleName);

  ResultTy visit(ast::root_statement_block *stm) override;
  ResultTy visit(ast::ArrayHolder *ArrStore) override;
  ResultTy visit(ast::ArrayAccessAssignment *Arr) override;
  ResultTy visit(ast::ArrayAccess *ArrAccess) override;
  ResultTy visit(ast::calc_expression *stm) override;
  ResultTy visit(ast::un_operator *stm) override;
  ResultTy visit(ast::logic_expression *stm) override;
  ResultTy visit(ast::number *stm) override;
  ResultTy visit(ast::variable *stm) override;
  ResultTy visit(ast::assignment *stm) override;
  ResultTy visit(ast::read_expression *stm) override;
  ResultTy visit(ast::statement_block *stm) override;
  ResultTy visit(ast::if_operator *stm) override;
  ResultTy visit(ast::while_operator *stm) override;
  ResultTy visit(ast::print_function *stm) override;
  ResultArrayTy visit(ast::PresetArray *PresetArr) override;
  ResultArrayTy visit(ast::UniformArray *UnifArr) override;

  // Generate LLVM IR and write it to Os
  void generateIRCode(ast::root_statement_block *RootBlock, raw_ostream &Os);
  void dumpInDotFormat(StringRef CFGName) const {
    codegen::dumpInDotFormat(*CodeGen.Mod.get(), CFGName);
  }

private:
  IRBuilder<> &Builder() { return *CodeGen.Builder.get(); }

  Module &Module() { return *CodeGen.Mod.get(); }

  ResultTy acceptASTNode(ast::statement *Stm) override {
    return static_cast<ResultTy>(Stm->accept(this));
  }
  
  ResultTy createWrapperRef(Value *Val = nullptr) {
    return VisitorBase::createWrapperRef<CodeGenValue>(Val);
  }
  
  DefaultResultTy acceptASTNodeDefault(ast::statement *Stm) {
    return static_cast<DefaultResultTy>(Stm->accept(this));
  }
  
  ResultArrayTy getOrCreateArrayInfo(DefaultResultTy DefRes, Value *Size);

  std::pair<BasicBlock *, BasicBlock *> createStartIf();
  void createEndIf(BasicBlock *EndBlock);

  std::pair<BasicBlock *, BasicBlock *> createStartWhile(Value *Condition);
  void createEndWhile(Value *Condition, BasicBlock *BodyBlock,
                      BasicBlock *EndBlock);

  // Generates an LLVM IR loop that increments a counter (LoopCounter) from 0 to
  // LoopLimit, invoking the provided CallLoopBody function in each iteration.
  // The loop counter is automatically created, managed, and passed as the first
  // argument to CallLoopBody, followed by any additional user-defined arguments
  // (Args). LoopCounter is passed to the CallLoopBody func because we often use
  // it in the body of the loop.
  template <typename... ArgsTy>
  void
  createUpCountLoop(Value *LoopLimit,
                    const std::function<void(Value *, ArgsTy...)> &CallLoopBody,
                    ArgsTy &&...Args) {
    auto *DataTy = CodeGen.getInt32Ty();
    Value *LoopCounter =
        createLocalVariable(DataTy, ConstantInt::get(DataTy, 0));
    auto *AllocaCounter = dyn_cast<LoadInst>(LoopCounter)->getPointerOperand();

    auto *Cond = Builder().CreateICmpSLT(LoopCounter, LoopLimit);
    auto [BodyWhile, EndWhile] = createStartWhile(Cond);
    LoopCounter = Builder().CreateLoad(DataTy, AllocaCounter);
    // Codegen the loop body
    CallLoopBody(LoopCounter, std::forward<ArgsTy>(Args)...);
    // Increment loop counter
    LoopCounter = Builder().CreateAdd(LoopCounter, ConstantInt::get(DataTy, 1));
    Builder().CreateStore(LoopCounter, AllocaCounter);
    Cond = Builder().CreateICmpSLT(LoopCounter, LoopLimit);
    createEndWhile(Cond, BodyWhile, EndWhile);
  }

  Value *createLogicAnd(ast::logic_expression *LogExp);
  Value *createLogicOr(ast::logic_expression *LogExp);

  Value *createArray(IntegerType *DataTy, const ArrayInfo &ArrInfo,
                     ast::statement_block *CurrScope);
  AllocaInst *allocateLocalArray(Type *DataTy, ArrayRef<Value *> Elems,
                                 unsigned ArrSize, unsigned ElemSize);

  Value *getArrayAccessPtr(ast::ArrayAccess *ArrAccess);

  LoadInst *createLocalVariable(Type *DataTy, Value *ToStore);

  void printIntegerValue(Value *Val);

  void printIRToOstream(raw_ostream &Os) const;

  void freeResources(ast::statement_block *StmBlock);

  SymTable<Type> SymTbl;
  ValueManager<Value> ValManager;
  IRCodeGenerator CodeGen;
  ArrayInfo CurrArrInfo;
  DenseMap<Value *, ArrayInfo> ArrInfoMap;
  ArrayManager ArrManager;
  DenseMap<ast::statement_block *, SmallVector<Value *>> ResourcesToFree;
};

} // namespace codegen
} // namespace paracl
