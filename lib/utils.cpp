#include <llvm/IR/Module.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/GraphWriter.h>
#include <llvm/Support/raw_ostream.h>

#include "codegen.hpp"
#include "utils.hpp"

namespace llvm {

template <>
struct DOTGraphTraits<const Function *> : public DefaultDOTGraphTraits {
  DOTGraphTraits(bool isSimple = false) : DefaultDOTGraphTraits(isSimple) {}

  static std::string getGraphName(const Function *F) {
    return llvm::formatv("CFG for '{0}' function", F->getName());
  }

  std::string getNodeLabel(const BasicBlock *Node, const Function *) {
    std::string OutStr;
    raw_string_ostream OSS(OutStr);

    if (isSimple()) {
      OSS << ": " << Node->getName();
    } else {
      Node->print(OSS);
    }

    if (OutStr[0] == '\n')
      OutStr.erase(OutStr.begin());
    return OutStr;
  }
};

} // namespace llvm

namespace paracl {

using namespace llvm;

namespace codegen {

void dumpInDotFormat(const Module &Mod, StringRef FileNameToDump) {
  const auto *StartFunc = Mod.getFunction(IRCodeGenerator::ParaCLStartFuncName);
  std::string NameStor;
  if (FileNameToDump.empty())
    FileNameToDump = "ParaclCFGDump.dot";
  else if (!FileNameToDump.ends_with(".dot")) {
    NameStor = FileNameToDump.str().append(".dot");
    FileNameToDump = NameStor;
  }

  std::error_code EC;
  raw_fd_ostream Os(FileNameToDump, EC);
  if (EC) {
    Os.close();
    paracl::fatal(llvm::formatv("error: {0}", EC.message()));
  }

  WriteGraph(Os, StartFunc, false, "Paracl CFG");
}

} // namespace codegen

[[noreturn]] void fatal(Twine ErrMes) {
  LLVMContext Ctx;
  ParaCLDiagnosticInfo Diag(llvm::DS_Error, ErrMes);
  Ctx.diagnose(Diag);
  llvm_unreachable("paracl::fatal should never return");
}

const int ParaCLDiagnosticInfo::KindID = getNextAvailablePluginDiagnosticKind();

} // namespace paracl
