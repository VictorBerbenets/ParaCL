#pragma once

#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/IR/LLVMContext.h"
#include <llvm/IR/Module.h>
#include <llvm/Support/FormatVariadic.h>

namespace paracl {

class ParaCLDiagnosticInfo : public llvm::DiagnosticInfo {
public:
  ParaCLDiagnosticInfo(llvm::DiagnosticSeverity Severity,
                       const llvm::Twine &Msg)
      : DiagnosticInfo(getKindID(), Severity), Msg(Msg.str()) {}

  void print(llvm::DiagnosticPrinter &DP) const override { DP << Msg; }

  static int getKindID() { return KindID; }

private:
  static const int KindID;
  std::string Msg;
};

namespace codegen {

void dumpInDotFormat(const llvm::Module &Mod, llvm::StringRef FileNameToDump);

} // namespace codegen

[[noreturn]] void fatal(llvm::Twine ErrMes);

} // namespace paracl
