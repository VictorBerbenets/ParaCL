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
  SmallVector<Type *> PrintParams{getInt32Ty()};
  auto *PrintFuncTy = FunctionType::get(getVoidTy(), PrintParams, false);
  Function::Create(PrintFuncTy, Function::ExternalLinkage, ParaCLPrintFuncName,
                   Mod.get());
  // Create __pcl_scan
  SmallVector<Type *> ScanParams{getVoidTy()};
  auto *ScanFuncTy = FunctionType::get(getInt32Ty(), ScanParams, false);
  Function::Create(ScanFuncTy, Function::ExternalLinkage, ParaCLScanFuncName,
                   Mod.get());
}

} // namespace codegen

} // namespace paracl
