#include "symbol_table.hpp"

namespace paracl {

bool symbol_table::has(const std::string &var_name) const {
  return names_.find(var_name) != names_.end();
}

void symbol_table::add(const std::string &var_name, int value) {
  names_.insert({var_name, value});
}

symbol_table::value_type &symbol_table::operator[](const std::string &name) {
  return names_[name];
}

} // namespace paracl
