#include <iostream>

#include <sstream>

#include <FlexLexer.h>

#include "driver.hpp"

// here we can return non-zero if lexing is not done inspite of EOF detected
//int yyFlexLexer::yywrap() { return 1; }

int main(int argc, char** argv) {
  std::string Input{std::istreambuf_iterator<char>{std::cin},
                    std::istreambuf_iterator<char>{}};
  my_yy::Driver DRV{};
  std::istringstream ISS{Input};
  DRV.switchInputStream(&ISS);
  DRV.parse();
}
