#include <iostream>
#include <sstream>
#include <FlexLexer.h>

#include "driver.hpp"
#include "ast.hpp"
#include "expression.hpp"

int main(int argc, char** argv) {
  std::ifstream i_stream{argv[1]};
  yy::driver driver{};

  driver.switch_input_stream(&i_stream);
  driver.parse();
  // driver.print_ast("ast.txt");
  driver.evaluate();
}
