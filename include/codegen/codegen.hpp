#pragma once

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <map>
#include <memory>

namespace paracl {

class CodeGenVisitor;

namespace codegen {

using namespace llvm;

// This class provides methods for generating paraCL code to LLVM IR.
class IRCodeGenerator final {
public:
  static constexpr StringRef ParaCLStartFuncName = "__pcl_start";
  static constexpr StringRef ParaCLPrintFuncName = "__pcl_print";
  static constexpr StringRef ParaCLScanFuncName = "__pcl_scan";

  IRCodeGenerator(StringRef ModuleName);

  Type *getInt32Ty();
  Type *getVoidTy();

  BasicBlock *createBasicBlock(Function *Func, StringRef Name);

  template <typename... ArgsTy,
            typename = std::enable_if_t<
                (... && std::is_convertible_v<Type *, ArgsTy>)>>
  Function *createFunction(Type *Ret, Function::LinkageTypes LinkType,
                           StringRef Name, bool IsVarArg, ArgsTy&&... Args) {
    std::array<Type *, sizeof...(ArgsTy)> FuncArgs{
        {std::forward<ArgsTy>(Args)...}};

    auto *FuncType = FunctionType::get(Ret, FuncArgs, IsVarArg);
    return Function::Create(FuncType, LinkType, Name, Mod.get());
  }

  Function *createFunction(Type *Ret, ArrayRef<Type *> Args,
                           Function::LinkageTypes LinkType, StringRef Name,
                           bool IsVarArg = false);

  friend class paracl::CodeGenVisitor;

private:
  // Create print and scan function decls
  void createParaCLStdLibFuncsDecls();

  LLVMContext Context;
  std::unique_ptr<IRBuilder<>> Builder;
  std::unique_ptr<Module> Mod;
  std::map<std::string, Value *> NamedValues;
};

} // namespace codegen

} // namespace paracl
