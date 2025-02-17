#include <llvm/Support/CommandLine.h>
#include <llvm/Support/ErrorHandling.h>

#include <FlexLexer.h>

#include <fstream>
#include <string>

#include "codegen.hpp"
#include "driver.hpp"

namespace {

namespace cl = llvm::cl;

enum OperatingMode { Compiler, Interpreter };

cl::opt<std::string> InputFileName(cl::Positional, cl::desc("<input file>"),
                                   cl::value_desc("filename"), cl::Required);

cl::opt<OperatingMode> OperatingMode(
    "operating-mode", cl::desc("Set the operating mode"), cl::init(Interpreter),
    cl::values(clEnumValN(Compiler, "compiler",
                          "Compiling paraCL code in llvm IR"),
               clEnumValN(Interpreter, "interpreter",
                          "Interpreting paraCL code without compiling")));

cl::opt<std::string>
    OutputFileName("o", cl::desc("Specify output filename for llvm IR"),
                   cl::value_desc("filename"), cl::init("paracl.ll"),
                   cl::Optional);
} // namespace

namespace paracl::codegen {
std::unique_ptr<IRCodeGenerator> CodeGen(nullptr);
} // namespace paracl::codegen

int main(int argc, char **argv) try {
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

  switch (OperatingMode) {
  case Interpreter:
    ParseDriver.evaluate();
    break;
  case Compiler:
    ParseDriver.compile(OutputFileName);
    break;
  default:
    llvm_unreachable("Wrong operating-mode for paraCL");
  }

} catch (const std::exception &Except) {
  llvm::errs() << "Exception catched: " << Except.what() << '\n';
} catch (...) {
  llvm::errs() << "Exception unknown" << '\n';
}
