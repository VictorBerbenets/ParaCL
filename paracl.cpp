#include <llvm/Support/CommandLine.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/raw_ostream.h>

#include <FlexLexer.h>

#include <fstream>
#include <string>

#include "codegen.hpp"
#include "driver.hpp"

namespace {

namespace cl = llvm::cl;

enum OperatingMode { Compiler, Interpreter };

cl::OptionCategory
    ParaCLCategory("ParaCL options",
                   "Options for controlling the running process.");

cl::opt<std::string> InputFileName(cl::Positional, cl::desc("<input file>"),
                                   cl::value_desc("filename"), cl::Required,
                                   cl::cat(ParaCLCategory));

cl::opt<std::string> ModuleName("module-name",
                                cl::desc("Set the name for the paraCL module"),
                                cl::value_desc("paraCL module name"),
                                cl::Optional, cl::init("pcl_module"),
                                cl::cat(ParaCLCategory));

cl::opt<OperatingMode> OperatingMode(
    "oper-mode", cl::desc("Set the operating mode"), cl::init(Interpreter),
    cl::values(clEnumValN(Compiler, "compiler",
                          "Compiling paraCL code in llvm IR"),
               clEnumValN(Interpreter, "interpreter",
                          "Interpreting paraCL code without compiling")),
    cl::Optional, cl::cat(ParaCLCategory));

cl::opt<std::string>
    OutputFileName("o", cl::desc("Specify output filename for llvm IR"),
                   cl::value_desc("filename"), cl::Optional,
                   cl::cat(ParaCLCategory));

void printParaCLVersion(llvm::raw_ostream &Os) { Os << "ParaCL: 1.0" << '\n'; }

} // namespace

int main(int argc, char **argv) try {
  cl::SetVersionPrinter(printParaCLVersion);
  cl::HideUnrelatedOptions(ParaCLCategory);
  cl::ParseCommandLineOptions(argc, argv,
                              " ParaCL (custom para C language))\n\n"
                              " This program has two modes: compiler in llvm "
                              "IR and interpreter (set on default).\n");
  std::ifstream InputFileStream(InputFileName);
  yy::driver ParseDriver;

  ParseDriver.switch_input_stream(&InputFileStream);
  ParseDriver.parse();

  if (auto errors = ParseDriver.check_for_errors(); errors) {
    llvm::errs() << "The program has been stopped. Found errors:" << '\n';
    errors.value().print_errors(std::cerr);
    return -1;
  }

  if (OperatingMode == Interpreter) {
    ParseDriver.evaluate();
  } else if (OperatingMode == Compiler) {
    if (!OutputFileName.getValue().empty()) {
      std::error_code ErrCode;
      llvm::raw_fd_ostream FileOs(OutputFileName, ErrCode);
      if (ErrCode)
        llvm::errs() << ErrCode.message().c_str() << "\n";
      ParseDriver.compile(ModuleName, FileOs);
      FileOs.close();
    } else {
      ParseDriver.compile(ModuleName, llvm::outs());
    }
  } else {
    llvm_unreachable("Unknown operating mode for paraCL");
  }

} catch (const std::exception &Except) {
  llvm::errs() << "Exception catched: " << Except.what() << '\n';
} catch (...) {
  llvm::errs() << "Exception unknown" << '\n';
}
