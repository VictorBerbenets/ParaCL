#include <gtest/gtest.h>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <FlexLexer.h>

#include "driver.hpp"
#include "ast.hpp"
#include "expression.hpp"

constexpr int lcm(int a, int b) {
    return 1;
}

TEST(LCM, test1) {
  std::ostringstream para_cl_ans;
  std::ostringstream cpp_ans;

  std::ifstream i_stream{"data/lcm.txt"};

  yy::driver driver{};

  driver.switch_input_stream(&i_stream);
  driver.parse();
  driver.evaluate(para_cl_ans, i_stream);
}