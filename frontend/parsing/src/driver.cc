#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <FlexLexer.h>

#include "driver.hpp"
#include "ast.hpp"
#include "expression.hpp"

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "paraCL: fatal error: no input file" << std::endl <<
                 "Program terminated" << std::endl;
    return -1;
  }

  std::ifstream i_stream{argv[1]};
  yy::driver driver{};

  driver.switch_input_stream(&i_stream);
  driver.parse();
  driver.print_ast("ast.txt");
  driver.evaluate();
}
