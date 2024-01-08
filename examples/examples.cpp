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

namespace paracl_testing {

auto get_paracl_ans(const std::string &input_name, std::istream &input_data = std::cin) {
  std::ostringstream para_cl_ans;
  std::ifstream i_stream {input_name};

  yy::driver driver{};

  driver.switch_input_stream(&i_stream);
  driver.parse();
  driver.evaluate(para_cl_ans, input_data);

  return para_cl_ans;
}

TEST(TEST1, PRINT) {
  std::ostringstream cpp_ans;

  for (int i = 10; i ; --i) {
    cpp_ans << i << std::endl;
  }

  ASSERT_EQ(cpp_ans.view(), get_paracl_ans("../examples/data/print.txt").view());
}


TEST(TEST2, SCAN) {
  std::ostringstream cpp_ans;
  std::stringstream input_data;
  
  std::cout << "Enter number" << std::endl;
  int tmp {};
  std::cin >> tmp;
  input_data << tmp;
  cpp_ans << tmp + 1 << std::endl;

  ASSERT_EQ(cpp_ans.view(), get_paracl_ans("../examples/data/scan.txt", input_data).view());
}

TEST(TEST3, LCM) {
  std::cout << "Enter two positive numbers to find LCM" << std::endl;
  int a, b;
  std::cin >> a >> b;

  std::ostringstream cpp_ans;
  std::stringstream input_data;
  input_data << a << ' ' << b << std::endl;

  cpp_ans << std::lcm(a, b) << std::endl;

  ASSERT_EQ(cpp_ans.view(), get_paracl_ans("../examples/data/lcm.txt", input_data).view());
}

TEST(TEST4, GCD) {
  std::cout << "Enter two positive numbers to find GCD" << std::endl;
  int a, b;
  std::cin >> a >> b;

  std::ostringstream cpp_ans;
  std::stringstream input_data;
  input_data << a << ' ' << b << std::endl;

  cpp_ans << std::gcd(a, b) << std::endl;

  ASSERT_EQ(cpp_ans.view(), get_paracl_ans("../examples/data/gcd.txt", input_data).view());
}

TEST(TEST5, FIBONAGHI) {
  std::cout << "Enter fibonachi number" << std::endl;
  int n;
  std::cin >> n;

  std::stringstream input_data;
  input_data << n << std::endl;

  std::ifstream i_stream {"../examples/data/fibonachi.txt"};
  std::ostringstream cpp_ans;

  auto fib = [](int n) {
    int a = 0, b = 1;
    for(int i = 2; i <= n; ++i) {
      int c = a + b;
      a = b;
      b = c;
    }
    return b;
  };

  cpp_ans << fib(n) << std::endl;

  ASSERT_EQ(cpp_ans.view(), get_paracl_ans("../examples/data/fibonachi.txt", input_data).view());
}

} // <--- namespace paracl_testing

