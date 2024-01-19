#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <FlexLexer.h>

#include "driver.hpp"

namespace paracl_testing {

inline std::ostringstream get_paracl_ans(const std::string &input_name,
                                         std::istream &input_data = std::cin) {
  std::ostringstream para_cl_ans;
  std::ifstream i_stream {input_name};

  yy::driver driver{};

  driver.switch_input_stream(&i_stream);
  driver.parse();
  driver.evaluate(para_cl_ans, input_data);

  return para_cl_ans;
}

} // <--- namespace paracl_testing


