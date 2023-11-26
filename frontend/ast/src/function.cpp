#include "function.hpp"

namespace frontend {

namespace ast {

scan_function::scan_function(const std::string& var_name)
    : var_name_ {var_name} {}

scan_function::scan_function(std::string&& var_name)
    : var_name_ {std::move(var_name)} {}

} // <--- namespace ast

} // <--- namespace frontend
