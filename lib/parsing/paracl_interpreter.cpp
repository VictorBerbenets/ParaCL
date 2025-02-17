#include <FlexLexer.h>
#include <filesystem>
#include <iostream>

#include "ast.hpp"
#include "driver.hpp"
#include "expression.hpp"

namespace fs = std::filesystem;

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "paraCL: fatal error: no input file"
              << " (it must be like that: ./paraCL filename)" << std::endl
              << "Program terminated" << std::endl;
    return -1;
  } else if (!fs::is_regular_file(argv[1])) {
    std::cerr << "paraCL: error: can't oppen file: " << argv[1] << std::endl;
    return -1;
  }

  std::ifstream i_stream{argv[1]};
  yy::driver driver{};

  driver.switch_input_stream(&i_stream);
  driver.parse();

  if (auto errors = driver.check_for_errors(); errors) {
    std::cerr << "The program has been stopped. Found errors:" << std::endl;
    errors.value().print_errors(std::cerr);
    return -1;
  }
  driver.evaluate();
}
