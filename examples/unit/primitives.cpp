#include <gtest/gtest.h>
#include <iostream>
#include <string>

#include "tests.hpp"

namespace paracl_testing {

static const std::string data_path = "../examples/unit/data/";

TEST(PRIMITIVES, SCOPE1) {
  get_paracl_ans(data_path + "scope1.txt");
}

TEST(PRIMITIVES, SCOPE2) {
  get_paracl_ans(data_path + "scope2.txt");
}

TEST(PRIMITIVES, SCOPE3) {
  get_paracl_ans(data_path + "scope3.txt");
}

TEST(PRIMITIVES, SCOLON1) {
  get_paracl_ans(data_path + "scolon1.txt");
}
TEST(PRIMITIVES, SCOLON2) {
  get_paracl_ans(data_path + "scolon2.txt");
}

} // <--- namespace paracl_testing

