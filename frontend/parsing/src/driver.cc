#include <iostream>
#include <sstream>
#include <FlexLexer.h>

#include "driver.hpp"
#include "ast.hpp"
#include "expression.hpp"

int main(int argc, char** argv) {
  std::string str_input{std::istreambuf_iterator<char>{std::cin},
                        std::istreambuf_iterator<char>{}};
  yy::Driver driver{};
  std::istringstream iss_str{str_input};
  driver.switchInputStream(&iss_str);
  driver.parse();
  using namespace frontend::ast;
  /*
  auto num = number(10);
  auto var = variable("string");
  auto ptr = make_node<bin_operator>(BinOp::ADD, num, var);
  */
  ast ast;
}
