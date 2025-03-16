#pragma once

#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Casting.h>

#include <concepts>

#include "codegen.hpp"
#include "semantic_context.hpp"
#include "visitor.hpp"

namespace paracl {

using namespace llvm;

template <typename ConstTy>
concept DerivedFromLLVMConstant = std::derived_from<ConstTy, Constant>;

class CodeGenVisitor : public VisitorBase {

  struct ArrayInfo final {
    SmallVector<Value *> Sizes;
    SmallVector<Value *> Data;

    bool isConstant() const { return isConstantData(Data) && isConstantData(Sizes); }

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

private:
  IRBuilder<> &Builder() { return *CodeGen.Builder.get(); }

  Module &Module() { return *CodeGen.Mod.get(); }

  static ConstantInt *isConstantInt(Value *Val) {
    return dyn_cast<ConstantInt>(Val);
  }

  Value *getCurrValue() const noexcept { return CurrVal; }
  Value *getValueAfterAccept(ast::statement *Stm);

  std::pair<BasicBlock *, BasicBlock *> createStartWhile(Value *Condition);
  void createEndWhile(Value *Condition, BasicBlock *BodyBlock,
                      BasicBlock *EndBlock);

  Value *createLogicAnd(ast::logic_expression *LogExp);
  Value *createLogicOr(ast::logic_expression *LogExp);

  Value *createArrayWithData(Type* DataTy, ArrayRef<Value *> Elems, unsigned ElemSize);

  void printIntegerValue(Value *Val);

  void setCurrValue(Value *Value) noexcept { CurrVal = Value; }

  void printIRToOstream(raw_ostream &Os) const;
    
  static std::optional<SmallVector<Constant *>> tryConvertDataToConstant(ArrayRef<Value *> Data) {
      SmallVector<Constant *> ConstData;
      ConstData.reserve(Data.size());
      if (!isConstantData(Data))
        return {};

      llvm::transform(Data, std::back_inserter(ConstData), [](auto *Val) {
        auto *ConstVal = dyn_cast<Constant>(Val);
        assert(ConstVal);
        return ConstVal;
      });
      return ConstData;
    }

    static void fillArrayWithData(IRBuilder<> &Builder, Value *ArrPtr, Type *DataTy, ArrayRef<Value*> Data) {
      for (unsigned Id = 0; Id < Data.size(); ++Id) {
        auto *GEPPtr =
            Builder.CreateGEP(DataTy, ArrPtr, ConstantInt::get(DataTy, Id));
        Builder.CreateStore(Data[Id], GEPPtr);
      }
    }

    static bool isConstantData(ArrayRef<Value *> Data) {
      return all_of(Data,
                    [](auto *Val) { return isConstantInt(Val) != nullptr; });
    }

  template <DerivedFromLLVMConstant ConstTy = Constant>
  ConstTy *isCompileTimeConstant(Value *Val) const {
    return dyn_cast<ConstTy>(Val);
  }

  Value *getArrayAccessPtr(ast::ArrayAccess *ArrAccess);

  Value *CurrVal;
  SymTable<Type> SymTbl;
  ValueManager<Value> ValManager;
  codegen::IRCodeGenerator CodeGen;
  ArrayInfo ArrInfo;
  llvm::DenseMap<Value *, ArrayInfo> ArrValueInfo;
};

} // namespace paracl
