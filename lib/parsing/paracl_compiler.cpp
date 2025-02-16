#include <llvm/Support/CommandLine.h>

#include <string>

namespace cl = llvm::cl;

cl::opt<std::string> InputFileName(cl::Positional, cl::desc("<input file>"),
                                   cl::value_desc("filename"), cl::Required);

cl::opt<std::string> OutputFileName("o", cl::desc("Specify output filename for llvm IR"),
                                     cl::value_desc("filename"), cl::init("paracl.ll"), cl::Optional);



int main(int argc, char** argv) try {
  cl::ParseCommandLineOptions(argc, argv, " ParaCL (custom compile program)\n\n"
                              " This program translates code from paraCL to llvm IR\n");

} catch (...) {

}

