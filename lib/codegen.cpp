#include <llvm/IR/GlobalValue.h>

#include "codegen.hpp"

namespace paracl {

namespace codegen {
  
IRCodeGenerator::IRCodeGenerator(StringRef ModuleName): Builder(std::make_unique<IRBuilder<>>(Context)),
                                                        Mod(std::make_unique<Module>(ModuleName, Context)) {
  createParaCLStdLibFuncsDecls();
}

Type *IRCodeGenerator::getInt32Ty() {
  return Type::getInt32Ty(Context);
}

Type *IRCodeGenerator::getVoidTy() {
  return Type::getVoidTy(Context);
}
  
void IRCodeGenerator::createParaCLStdLibFuncsDecls() {
  // Create __pcl_print
  ArrayRef<Type*> PrintParams (getInt32Ty());
  auto *PrintFuncTy = FunctionType::get(getVoidTy(), PrintParams, false); 
  Function::Create(PrintFuncTy, Function::ExternalLinkage, ParaCLPrintFuncName, Mod.get());
  // Create __pcl_scan
  ArrayRef<Type*> ScanParams (getVoidTy());
  auto *ScanFuncTy = FunctionType::get(getInt32Ty(), ScanParams, false); 
  Function::Create(ScanFuncTy, Function::ExternalLinkage, ParaCLScanFuncName, Mod.get());
}

std::unique_ptr<IRCodeGenerator> createIRCodeGenerator(llvm::StringRef Name) {
  auto Gen = std::make_unique<IRCodeGenerator>(Name);
  // Add standart paraCL library functions

  return Gen;
}

} // namespace codegen

} // paracl
