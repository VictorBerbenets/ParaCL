#include "function.hpp"
#include "visitor.hpp"

#include <iostream>

namespace frontend {

namespace ast {

scan_function::scan_function(const std::string& var_name)
    : var_name_ {var_name} {}

scan_function::scan_function(std::string&& var_name)
    : var_name_ {std::move(var_name)} {}

void scan_function::accept(base_visitor *b_visitor) {
  b_visitor->visit(this);
}

void print_function::accept(base_visitor *b_visitor) {
    // std::cout << "PRINTING\n";
  b_visitor->visit(this);
}

} // <--- namespace ast

} // <--- namespace frontend
