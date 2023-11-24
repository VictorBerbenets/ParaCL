#include <iostream>
#include <sstream>
#include <FlexLexer.h>

#include "driver.hpp"


int main(int argc, char** argv) {
  std::string Input{std::istreambuf_iterator<char>{std::cin},
                    std::istreambuf_iterator<char>{}};
  my_yy::Driver DRV{};
  std::istringstream ISS{Input};
  DRV.switchInputStream(&ISS);
  DRV.parse();
}
