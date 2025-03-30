#include <llvm/Support/CommandLine.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/raw_ostream.h>

#include <FlexLexer.h>
#include <fstream>
#include <string>

#include "driver.hpp"
#include "option_category.hpp"
#include "utils.hpp"

namespace {

namespace cl = llvm::cl;

enum Error { SyntaxErr = 0xbad2bad };
enum OperatingMode { Compiler, Interpreter };

cl::opt<std::string> InputFileName(cl::Positional, cl::desc("<input file>"),
                                   cl::value_desc("filename"), cl::Required,
                                   cl::cat(paracl::ParaCLCategory));

cl::opt<std::string> ModuleName("module-name",
                                cl::desc("Set the name for the paraCL module"),
                                cl::value_desc("paraCL module name"),
                                cl::Optional, cl::init("pcl_module"),
                                cl::cat(paracl::ParaCLCategory));

cl::opt<OperatingMode> OperatingMode(
    "oper-mode", cl::desc("Set the operating mode"), cl::init(Interpreter),
    cl::values(clEnumValN(Compiler, "compiler",
                          "Compiling paraCL code in llvm IR"),
               clEnumValN(Interpreter, "interpreter",
                          "Interpreting paraCL code without compiling")),
    cl::Optional, cl::cat(paracl::ParaCLCategory));

cl::opt<std::string>
    OutputFileName("o", cl::desc("Specify output filename for llvm IR"),
                   cl::value_desc("filename"), cl::Optional,
                   cl::cat(paracl::ParaCLCategory));

void printParaCLVersion(llvm::raw_ostream &Os) { Os << "ParaCL: 1.0" << '\n'; }

} // namespace

int main(int argc, char **argv) try {
  cl::SetVersionPrinter(printParaCLVersion);
  cl::HideUnrelatedOptions(paracl::ParaCLCategory);
  cl::ParseCommandLineOptions(argc, argv,
                              " ParaCL (custom para C language))\n\n"
                              " This program has two modes: compiler in llvm "
                              "IR and interpreter (set on default).\n");
  std::ifstream InputFileStream(InputFileName);
  if (InputFileStream.fail())
    paracl::fatal(llvm::formatv("no such file: '{0}'\n", InputFileName));

  yy::driver Driver;
  Driver.switch_input_stream(&InputFileStream);
  Driver.parse();
  if (auto errors = Driver.validate(); errors) {
    errors.value().print_errors(llvm::errs(), InputFileName);
    return SyntaxErr;
  }

  if (OperatingMode == Compiler) {
    if (OutputFileName.getNumOccurrences() > 0) {
      std::error_code ErrCode;
      llvm::raw_fd_ostream FileOs(OutputFileName, ErrCode);
      if (ErrCode)
        paracl::fatal(ErrCode.message().c_str());

      Driver.compile(ModuleName, FileOs);
      FileOs.close();
    } else {
      Driver.compile(ModuleName, llvm::outs());
    }
  } else if (OperatingMode == Interpreter) {
    Driver.evaluate();
  } else {
    llvm_unreachable("Unknown operating mode for paraCL");
  }

} catch (const std::exception &Except) {
  llvm::errs() << "Exception catched: " << Except.what() << '\n';
} catch (...) {
  llvm::errs() << "Exception unknown" << '\n';
}
