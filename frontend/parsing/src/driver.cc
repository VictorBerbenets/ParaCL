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
    throw std::runtime_error{"invalid arguments number: expected 2, "
                             "got " + std::to_string(argc)};
  }

  std::ifstream i_stream{argv[1]};
  yy::driver driver{};

  driver.switch_input_stream(&i_stream);
  driver.parse();
  driver.evaluate();
}
