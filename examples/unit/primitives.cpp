#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <numeric>
#include <FlexLexer.h>

#include "driver.hpp"

namespace paracl_testing {

static const std::string data_path = "../examples/unit/data/";

auto get_paracl_ans(const std::string &input_name, std::istream &input_data = std::cin) {
  std::ostringstream para_cl_ans;
  std::ifstream i_stream {input_name};

  yy::driver driver{};

  driver.switch_input_stream(&i_stream);
  driver.parse();
  driver.evaluate(para_cl_ans, input_data);

  return para_cl_ans;
}


TEST(PRIMITIVES, SCOPE1) {
  get_paracl_ans(data_path + "scope.txt");
}

} // <--- namespace paracl_testing

