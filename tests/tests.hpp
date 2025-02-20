#pragma once

#include <FlexLexer.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <vector>

#include "driver.hpp"

namespace paracl_testing {

inline std::ostringstream get_paracl_ans(const std::string &input_name,
                                         std::istream &input_data = std::cin) {
  std::ostringstream para_cl_ans;
  std::ifstream i_stream{input_name};

  yy::driver driver{};

  driver.switch_input_stream(&i_stream);
  driver.parse();
  driver.evaluate(para_cl_ans, input_data);

  return para_cl_ans;
}

template <typename T = int>
std::vector<T> random_data(std::size_t data_size, T low_val, T max_val) {
  std::random_device dev;
  std::mt19937 engine{dev()};
  std::uniform_int_distribution<T> dist{low_val, max_val};

  std::vector<int> vec(data_size);
  std::generate(vec.begin(), vec.end(),
                [&dist, &engine]() { return dist(engine); });
  return vec;
}

template <std::input_iterator InputIt>
void print_to(std::ostream &os, InputIt begin, InputIt end) {
  std::for_each(begin, end, [&os](auto &&val) { os << val << std::endl; });
}

} // namespace paracl_testing
