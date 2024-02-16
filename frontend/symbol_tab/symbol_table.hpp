#pragma once

#include <unordered_map>
#include <memory>

#include "base_type.hpp"

namespace frontend {

class symbol_table {
  using size_type  = std::size_t;
  using value_type = ast::object_type*;
 public:

  bool has(const std::string &var_name) const {
    return objects_.find(var_name) != objects_.end();
  }

  // template <typename... Args>
  void add(value_type obj_ptr ) {
    objects_.insert({obj_ptr->name(), obj_ptr});
  }

  value_type &operator[](const std::string &name) {
    return objects_[name];
  }

 private:
  std::unordered_map<std::string, value_type> objects_;
};

} // <--- namespace frontend
