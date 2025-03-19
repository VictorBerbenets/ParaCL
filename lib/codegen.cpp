#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/TargetParser/Host.h>

#include "codegen.hpp"
#include "option_category.hpp"

namespace paracl {
namespace codegen {

cl::opt<std::string> TargetTriple("target-triple",
                                  cl::desc("Set the platform target triple"),
                                  cl::Optional,
                                  cl::init(llvm::sys::getDefaultTargetTriple()),
                                  cl::cat(ParaCLCategory));

IRCodeGenerator::IRCodeGenerator(StringRef ModuleName)
    : Builder(std::make_unique<IRBuilder<>>(Context)),
      Mod(std::make_unique<Module>(ModuleName, Context)) {
  assert(!TargetTriple.getValue().empty());
  Mod->setTargetTriple(TargetTriple.getValue());
  createParaCLStdLibFuncsDecls();
}

IntegerType *IRCodeGenerator::getInt32Ty() { return Type::getInt32Ty(Context); }

IntegerType *IRCodeGenerator::getInt1Ty() { return Type::getInt1Ty(Context); }

Type *IRCodeGenerator::getVoidTy() { return Type::getVoidTy(Context); }

Value *IRCodeGenerator::createCondValueIfNeed(Value *Val) {
  if (auto *Type = dyn_cast<IntegerType>(Val->getType());
      Type && Type->getBitWidth() == 1) {
    return Val;
  }

  auto *ZeroVal = createConstantInt32(0);
  return Builder->CreateICmpNE(Val, ZeroVal);
}

ConstantInt *IRCodeGenerator::createConstantInt32(unsigned Val, bool IsSigned) {
  return ConstantInt::get(getInt32Ty(), Val, IsSigned);
}

void IRCodeGenerator::createParaCLStdLibFuncsDecls() {
  // Create __pcl_print
  createFunction(getVoidTy(), Function::ExternalLinkage, ParaCLPrintFuncName,
                 false, getInt32Ty());
  // Create __pcl_scan
  createFunction(getInt32Ty(), Function::ExternalLinkage, ParaCLScanFuncName,
                 false);
}

Function *IRCodeGenerator::createFunction(Type *Ret, ArrayRef<Type *> Args,
                                          Function::LinkageTypes LinkType,
                                          StringRef Name, bool IsVarArg) {
  auto *FuncType = FunctionType::get(Ret, Args, IsVarArg);
  return Function::Create(FuncType, LinkType, Name, Mod.get());
}

BasicBlock *IRCodeGenerator::createBlockAndLinkWith(BasicBlock *CurrBlock,
                                                    StringRef Name,
                                                    bool InsertInNewBlock) {
  assert(CurrBlock);
  auto *NewBlock = BasicBlock::Create(Context, Name, CurrBlock->getParent());
  Builder->CreateBr(NewBlock);
  if (InsertInNewBlock)
    Builder->SetInsertPoint(NewBlock);

  return NewBlock;
}

const Module &IRCodeGenerator::getModule() const { return *Mod.get(); }

} // namespace codegen
} // namespace paracl
