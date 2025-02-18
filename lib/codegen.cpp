#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/GlobalValue.h>

#include "codegen.hpp"

namespace paracl {

namespace codegen {

IRCodeGenerator::IRCodeGenerator(StringRef ModuleName)
    : Builder(std::make_unique<IRBuilder<>>(Context)),
      Mod(std::make_unique<Module>(ModuleName, Context)) {
  createParaCLStdLibFuncsDecls();
}

Type *IRCodeGenerator::getInt32Ty() { return Type::getInt32Ty(Context); }

Type *IRCodeGenerator::getVoidTy() { return Type::getVoidTy(Context); }

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

} // namespace codegen

} // namespace paracl
