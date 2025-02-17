#pragma once

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/ADT/DenseMap.h>

#include <memory>
#include <map>

namespace paracl {

namespace codegen {

using namespace llvm;

// This class provides methods for generating paraCL code to LLVM IR.
class IRCodeGenerator final {

  static constexpr StringRef ParaCLStartFuncName = "__pcl_start";
  static constexpr StringRef ParaCLPrintFuncName = "__pcl_print";
  static constexpr StringRef ParaCLScanFuncName = "__pcl_scan";

public:
  IRCodeGenerator(StringRef ModuleName);


  Type *getInt32Ty();
  Type *getVoidTy();

   
private:
  void createParaCLStdLibFuncsDecls();

  LLVMContext Context;
  std::unique_ptr<IRBuilder<>> Builder;
  std::unique_ptr<Module> Mod;
  std::map<std::string, Value *> NamedValues;
};

extern std::unique_ptr<IRCodeGenerator> CodeGen;

std::unique_ptr<IRCodeGenerator> createIRCodeGenerator(std::string Name);

} // namespace codegen

} // namespace paracl
