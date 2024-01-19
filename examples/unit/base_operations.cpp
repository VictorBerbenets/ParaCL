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

} // <--- namespace paracl_testing

