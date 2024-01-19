#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <random>
#include <vector>
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

template <typename T = int>
std::vector<T> random_data(std::size_t data_size, T low_val, T max_val) {
  std::random_device dev;
  std::mt19937 engine {dev()};
  std::uniform_int_distribution<T> dist {low_val, max_val};

  std::vector<int> vec(data_size);
  std::generate(begin(vec), end(vec), [&dist, &engine]() {
                                        return dist(engine); 
                                      } );
  return vec;
}

template <typename Container>
void print_to(std::ostream &os, const Container &cont) {
  for (auto &&val: cont) {
    os << val << std::endl;
  }
}

} // <--- namespace paracl_testing


