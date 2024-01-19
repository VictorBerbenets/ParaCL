#include <gtest/gtest.h>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <numeric>

#include "tests.hpp"

namespace paracl_testing {

static const std::string data_path = "../examples/interactive_tests/data/";

TEST(TEST1, PRINT) {
  std::ostringstream cpp_ans;

  for (int i = 10; i ; --i) {
    cpp_ans << i << std::endl;
  }

  ASSERT_EQ(cpp_ans.view(), get_paracl_ans(data_path + "print.txt").view());
}


TEST(TEST2, SCAN) {
  std::ostringstream cpp_ans;
  std::stringstream input_data;
  
  std::cout << "Enter number" << std::endl;
  int tmp {};
  std::cin >> tmp;
  input_data << tmp;
  cpp_ans << tmp + 1 << std::endl;

  ASSERT_EQ(cpp_ans.view(), get_paracl_ans(data_path + "scan.txt", input_data).view());
}

TEST(TEST3, LCM) {
  std::cout << "Enter two positive numbers to find LCM" << std::endl;
  int a, b;
  std::cin >> a >> b;

  std::ostringstream cpp_ans;
  std::stringstream input_data;
  input_data << a << ' ' << b << std::endl;

  auto ans = std::lcm(a, b);
  cpp_ans << ans << std::endl;
  std::cout << "correct answer is " << ans << std::endl;

  ASSERT_EQ(cpp_ans.view(), get_paracl_ans(data_path + "lcm.txt", input_data).view());
}

TEST(TEST4, GCD) {
  std::cout << "Enter two positive numbers to find GCD" << std::endl;
  int a, b;
  std::cin >> a >> b;

  std::ostringstream cpp_ans;
  std::stringstream input_data;
  input_data << a << ' ' << b << std::endl;
  
  auto ans = std::gcd(a, b);
  cpp_ans << ans << std::endl;
  std::cout << "correct answer is " << ans << std::endl;

  ASSERT_EQ(cpp_ans.view(), get_paracl_ans(data_path + "gcd.txt", input_data).view());
}

TEST(TEST5, FIBONAGHI) {
  std::cout << "Enter fibonachi number" << std::endl;
  int n;
  std::cin >> n;

  std::stringstream input_data;
  input_data << n << std::endl;

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
  
  auto ans = fib(n);
  cpp_ans << ans << std::endl;
  std::cout << "correct answer is " << ans << std::endl;

  ASSERT_EQ(cpp_ans.view(), get_paracl_ans(data_path + "fibonachi.txt", input_data).view());
}

TEST(TEST6, PRIME_FACTORS) {
  std::cout << "Enter number for finding its prime factors" << std::endl;
  int n;
  std::cin >> n;

  std::stringstream input_data;
  input_data << n << std::endl;

  std::ostringstream cpp_ans;

  auto print_prime_factors = [](std::ostream &os, int n) {
    int div = 2;
    while (n > 1)
    {
      while (n % div == 0)
      {
        os << div << std::endl;
        n = n / div;
      }
      div++;
    }
  };

  print_prime_factors(cpp_ans, n);
  ASSERT_EQ(cpp_ans.view(), get_paracl_ans(data_path + "prime_factors.txt", input_data).view());

  std::cout << "prime factors of " << n << ": " << std::endl;
  print_prime_factors(std::cout, n);
}

} // <--- namespace paracl_testing

