#pragma once

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Value.h>
#include <llvm/ADT/DenseMap.h>

#include <memory>
#include <map>

namespace paracl {

namespace codegen {

class IRCodeGenerator final {
  
private:
  llvm::LLVMContext Context;
  std::unique_ptr<llvm::IRBuilder<>> Builder;
  std::unique_ptr<llvm::Module> Module;
  std::map<std::string, llvm::Value *> NamedValues;
};

extern std::unique_ptr<IRCodeGenerator> CodeGen;

} // namespace codegen

} // namespace paracl
