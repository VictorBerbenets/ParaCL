#include <iostream>
#include <sstream>
#include <FlexLexer.h>

#include "driver.hpp"


int main(int argc, char** argv) {
  std::string str_input{std::istreambuf_iterator<char>{std::cin},
                        std::istreambuf_iterator<char>{}};
  yy::Driver driver{};
  std::istringstream iss_str{str_input};
  driver.switchInputStream(&iss_str);
  driver.parse();
}
