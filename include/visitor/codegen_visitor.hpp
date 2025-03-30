#pragma once

#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Casting.h>

#include <concepts>
#include <functional>

#include "codegen.hpp"
#include "semantic_context.hpp"
#include "utils.hpp"
#include "visitor.hpp"

namespace paracl {

using namespace llvm;

template <typename ConstTy>
concept DerivedFromLLVMConstant = std::derived_from<ConstTy, Constant>;

class CodeGenVisitor : public VisitorBase {
  // An auxiliary structure for recursively collecting data from an entire array
  // and preparing for its creation
  struct ArrayInfo final {
    SmallVector<Value *> Sizes;
    SmallVector<Value *> Data;

    bool isConstant() const {
      return isConstantData(Data) && isConstantData(Sizes);
    }

    void pushSize(Value *Sz) { Sizes.push_back(Sz); }

    void pushData(Value *Dat) { Data.push_back(Dat); }

    template <std::input_iterator InputIt>
    void pushData(InputIt Begin, InputIt End) {
      Data.insert(Data.end(), Begin, End);
    }

    void clearSize() { Sizes.clear(); }

    void clearData() { Data.clear(); }

    void clear() {
      clearSize();
      clearData();
    }

    static Value *calculateSize(IRBuilder<> &Builder, IntegerType *DataTy,
                                ArrayRef<Value *> Data) {
      if (auto OptData = tryConvertDataToConstant<ConstantInt>(Data);
          OptData.has_value())
        return calculateSize(DataTy, OptData.value());

      Value *ArrSize = ConstantInt::get(DataTy, 1);
      llvm::for_each(
          Data, [&](auto *Sz) { ArrSize = Builder.CreateMul(ArrSize, Sz); });
      return ArrSize;
    }

    static ConstantInt *calculateSize(IntegerType *DataTy,
                                      ArrayRef<ConstantInt *> Data) {
      auto *ArrSize = ConstantInt::get(DataTy, 1);
      llvm::for_each(Data, [&ArrSize](auto *Sz) {
        ArrSize = dyn_cast<ConstantInt>(ConstantExpr::getMul(ArrSize, Sz));
        assert(ArrSize);
      });
      return ArrSize;
    }
  };

public:
  CodeGenVisitor(StringRef ModuleName);

  void visit(ast::root_statement_block *stm) override;

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
  void generateIRCode(ast::root_statement_block *RootBlock, raw_ostream &Os);
  void dumpInDotFormat(StringRef CFGName) const {
    codegen::dumpInDotFormat(*CodeGen.Mod.get(), CFGName);
  }

private:
  IRBuilder<> &Builder() { return *CodeGen.Builder.get(); }

  Module &Module() { return *CodeGen.Mod.get(); }

  void setCurrValue(Value *Value) noexcept { CurrVal = Value; }

  Value *getCurrValue() const noexcept { return CurrVal; }
  Value *getValueAfterAccept(ast::statement *Stm);

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

  AllocaInst *createArrayWithData(Type *DataTy, ArrayRef<Value *> Elems,
                                  unsigned ArrSize, unsigned ElemSize);

  Value *getArrayAccessPtr(ast::ArrayAccess *ArrAccess);

  LoadInst *createLocalVariable(Type *DataTy, Value *ToStore);

  void printIntegerValue(Value *Val);

  void printIRToOstream(raw_ostream &Os) const;

  void freeResources(ast::statement_block *StmBlock);

  template <DerivedFromLLVMConstant ConstType = Constant>
  static std::optional<SmallVector<ConstType *>>
  tryConvertDataToConstant(ArrayRef<Value *> Data) {
    SmallVector<ConstType *> ConstData;
    ConstData.reserve(Data.size());
    if (!isConstantData(Data))
      return {};

    llvm::transform(Data, std::back_inserter(ConstData), [](auto *Val) {
      auto *ConstVal = dyn_cast<ConstType>(Val);
      assert(ConstVal);
      return ConstVal;
    });
    return ConstData;
  }

  static void fillArrayWithData(IRBuilder<> &Builder, Value *ArrPtr,
                                Type *DataTy, ArrayRef<Value *> Data);

  static bool isConstantData(ArrayRef<Value *> Data);

  static ConstantInt *isConstantInt(Value *Val);

  template <DerivedFromLLVMConstant ConstTy = Constant>
  static ConstTy *isCompileTimeConstant(Value *Val) {
    return dyn_cast<ConstTy>(Val);
  }

  Value *CurrVal;
  SymTable<Type> SymTbl;
  ValueManager<Value> ValManager;
  codegen::IRCodeGenerator CodeGen;

  ArrayInfo CurrArrInfo;
  DenseMap<Value *, ArrayInfo> ArrInfoMap;
  DenseMap<ast::statement_block *, SmallVector<Value *>> ResourcesToFree;
};

} // namespace paracl
