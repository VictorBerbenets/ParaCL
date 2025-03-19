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
namespace codegen {

void dumpInDotFormat(const llvm::Module &Mod, llvm::StringRef FileNameToDump) {
  const auto *StartFunc = Mod.getFunction(IRCodeGenerator::ParaCLStartFuncName);
  std::string NameStor;
  if (FileNameToDump.empty())
    FileNameToDump = "ParaclCFGDump.dot";
  else if (!FileNameToDump.ends_with(".dot")) {
    NameStor = FileNameToDump.str().append(".dot");
    FileNameToDump = NameStor;
  }

  std::error_code EC;
  llvm::raw_fd_ostream Os(FileNameToDump, EC);
  if (EC) {
    Os.close();
    throw std::runtime_error(llvm::formatv("error: {0}", EC.message()));
  }

  llvm::WriteGraph(Os, StartFunc, false, "Paracl CFG");
}

} // namespace codegen
} // namespace paracl
