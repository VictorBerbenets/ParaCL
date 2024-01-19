#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <sstream>

#include "tests.hpp"

namespace paracl_testing {

static const std::string data_path = "../examples/unit/data/";

TEST(CTRL_STATEMENTS, IF) {
  std::ostringstream ans;
  std::stringstream input_data;
  auto data = random_data(2, -10, 10);
  input_data << data[0] << ' ' << data[1] << std::endl;

  auto a = data[0];
  auto b = data[1];
// testing if part
  if (a + 1 < b - 1) {
    ans << a << std::endl;
  } else if (a * 10 > b * 5) {
    ans << b << std::endl;
  } else {
    a = a + b;
    ans << a + b << std::endl;
  }

  if (a == b) {
    ans << a * b << std::endl;
  } else if (b != 0) {
    ans << a / b << std::endl;
  } else {
    ans << 1000 << std::endl;
  }
  
  ASSERT_EQ(ans.view(), get_paracl_ans(data_path + "if.txt", input_data).view());
}

TEST(CTRL_STATEMENTS, WHILE) {
  std::ostringstream ans;
  std::stringstream input_data;
  auto data = random_data(1, -10000, 10000);
  input_data << data[0] << std::endl;

  auto a = data[0];
// testing while part
  while(a) {
    if (a > 0) {
      a = a - 1;
    } else {
      a = a + 1;
    }
    if (a + 1 == 5 || a - 1 == -5)
      ans << 5 << std::endl;
    ans << a << std::endl;
  }
 
  ASSERT_EQ(ans.view(), get_paracl_ans(data_path + "while.txt", input_data).view());
}

TEST(CTRL_STATEMENTS, WHILE_IF) {
  std::ostringstream ans;
  std::stringstream input_data;
  auto data = random_data(1, -1000, 1000);
  input_data << data[0] << std::endl;

  auto a = data[0];
// testing while part
  int b = 0;
  while (a > 0) {
      ans << a << std::endl;
      if (a / 2 == a - 1) {
          ans << 10 << std::endl;
      }
      while (b < a) {
          ans << b << std::endl;
          b = b + 1;
      }
      a = a - 1;
      b = 0;
  }

  ans << a << std::endl;
 
  ASSERT_EQ(ans.view(), get_paracl_ans(data_path + "while_if.txt", input_data).view());
}

} // <--- namespace paracl_testing

