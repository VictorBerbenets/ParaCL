#pragma once

#include <memory>
#include <unordered_map>

namespace paracl {

class symbol_table {
  using size_type = std::size_t;
  using value_type = int;

public:
  bool has(const std::string &var_name) const {
    return names_.find(var_name) != names_.end();
  }

  void add(const std::string &var_name, int value) {
    names_.insert({var_name, value});
  }

  value_type &operator[](const std::string &name) { return names_[name]; }

private:
  std::unordered_map<std::string, int> names_;
};

} // namespace paracl
