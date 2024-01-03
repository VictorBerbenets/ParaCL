#include <gtest/gtest.h>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <numeric>
#include <FlexLexer.h>

#include "driver.hpp"
#include "ast.hpp"
#include "expression.hpp"

TEST(TEST1, PRINT) {
  std::ostringstream para_cl_ans;
  std::ostringstream cpp_ans;

  for (int i = 10; i ; --i) {
    cpp_ans << i << std::endl;
  }

  std::ifstream i_stream{"../examples/data/print.txt"};

  yy::driver driver{};

  driver.switch_input_stream(&i_stream);
  driver.parse();
  driver.evaluate(para_cl_ans);

  ASSERT_EQ(cpp_ans.view(), para_cl_ans.view());
}


TEST(TEST2, SCAN) {
  std::ostringstream para_cl_ans;
  std::ostringstream cpp_ans;
  std::stringstream input_data;

  int tmp {};
  std::cin >> tmp;
  input_data << tmp;
  cpp_ans << tmp + 1 << std::endl;

  std::ifstream i_stream{"../examples/data/scan.txt"};

  yy::driver driver{};

  driver.switch_input_stream(&i_stream);
  driver.parse();
  driver.evaluate(para_cl_ans, input_data);

  ASSERT_EQ(cpp_ans.view(), para_cl_ans.view());
}

TEST(TEST3, LCM) {
  int a, b;
  std::cin >> a >> b;

  std::ostringstream para_cl_ans;
  std::ostringstream cpp_ans;
  std::stringstream input_data;
  input_data << a << ' ' << b << std::endl;

  cpp_ans << std::lcm(a, b) << std::endl;

  std::ifstream i_stream{"../examples/data/lcm.txt"};

  yy::driver driver{};

  driver.switch_input_stream(&i_stream);
  driver.parse();
  driver.evaluate(para_cl_ans, input_data);

  ASSERT_EQ(cpp_ans.view(), para_cl_ans.view());
}

TEST(TEST4, GCD) {
  int a, b;
  std::cin >> a >> b;

  std::ostringstream para_cl_ans;
  std::ostringstream cpp_ans;
  std::stringstream input_data;
  input_data << a << ' ' << b << std::endl;

  cpp_ans << std::gcd(a, b) << std::endl;

  std::ifstream i_stream{"../examples/data/gcd.txt"};

  yy::driver driver{};

  driver.switch_input_stream(&i_stream);
  driver.parse();
  driver.evaluate(para_cl_ans, input_data);

  ASSERT_EQ(cpp_ans.view(), para_cl_ans.view());
}
