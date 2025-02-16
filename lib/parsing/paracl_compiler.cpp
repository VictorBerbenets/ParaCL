#include <llvm/Support/CommandLine.h>
#include <FlexLexer.h>

#include "codegen.hpp"

#include "driver.hpp"

#include <string>
#include <fstream>

namespace {

namespace cl = llvm::cl;

enum OperatingMode { Compiler, Interpreter };

cl::opt<std::string> InputFileName(cl::Positional, cl::desc("<input file>"),
                                   cl::value_desc("filename"), cl::Required);

cl::opt<OperatingMode> OperatingMode("operating-mode", cl::desc("set the operating mode"), cl::init(Interpreter),
                                      cl::values(clEnumValN(Compiler, "compiler", "compiling paraCL code in llvm IR"),
                                                 clEnumValN(Interpreter, "interpreter", "interpreting paraCL code without compiling")));

cl::opt<std::string> OutputFileName("o", cl::desc("Specify output filename for llvm IR"),
                                     cl::value_desc("filename"), cl::init("paracl.ll"), cl::Optional);
} // namespace

namespace paracl::codegen {
  std::unique_ptr<IRCodeGenerator> CodeGen(nullptr);
} // codegen


int main(int argc, char** argv) try {
  cl::ParseCommandLineOptions(argc, argv, " ParaCL (custom para C language))\n\n"
                              " This program has two modes: compiler in llvm IR and interpreter (set on default).\n");
  std::ifstream InputFileStream(InputFileName);
  yy::driver ParseDriver;
  
  if (OperatingMode == Interpreter)
  ParseDriver.switch_input_stream(&InputFileStream);
  ParseDriver.parse();

  if (auto errors = ParseDriver.check_for_errors(); errors) {
    std::cerr << "The program has been stopped. Found errors:" << std::endl;
    errors.value().print_errors(std::cerr);
    return -1;
  }
  ParseDriver.evaluate();


} catch (...) {

}

