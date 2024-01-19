#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <sstream>

#include "tests.hpp"

namespace paracl_testing {

static const std::string data_path = "../examples/unit/data/";

TEST(BASE_OPERATIONS, ASSIGNMENT1) {
  std::ostringstream answer;
  answer << 5 << std::endl <<
            5 << std::endl <<
            4 << std::endl; 
  ASSERT_EQ(answer.view(), get_paracl_ans(data_path + "simple_assignment.txt").view());
}

TEST(BASE_OPERATIONS, ASSIGNMENT2) {
  std::ostringstream answer;
  answer << -1350 << std::endl <<
            -1500 << std::endl <<
            -150 << std::endl; 
  ASSERT_EQ(answer.view(), get_paracl_ans(data_path + "chainable_assignment.txt").view());
  
}

TEST(BASE_OPERATIONS, ARIPHMETIC) {
  std::ostringstream answer;
  answer << 6 << std::endl <<
            8 << std::endl <<
            6 << std::endl << 
            -2 << std::endl << 
            0 << std::endl << 
            20 << std::endl; 
  ASSERT_EQ(answer.view(), get_paracl_ans(data_path + "ariphmetic.txt").view());
  
}

TEST(BASE_OPERATIONS, SCAN1) {
  std::ostringstream ans;
  std::stringstream input_data;
  auto data = random_data(2, -100000, 100000);
  print_to(input_data, data);

  ans << data[0] << std::endl <<
         data[1] + data[0] << std::endl;

  ASSERT_EQ(ans.view(), get_paracl_ans(data_path + "scan1.txt", input_data).view());
}

TEST(BASE_OPERATIONS, SCAN2) {
  std::ostringstream ans;
  std::stringstream input_data;
  auto data = random_data(2, -1, 1);
  print_to(input_data, data);
  
  if (data[0]) {
    if (data[1]) {
      ans << 3 << std::endl;
    } else {
      ans << 2 << std::endl;
    }
  } else if (data[1]) {
    ans << 1 << std::endl;
  } else {
    ans << 0 << std::endl;
  }

  ASSERT_EQ(ans.view(), get_paracl_ans(data_path + "scan2.txt", input_data).view());
}

} // <--- namespace paracl_testing

