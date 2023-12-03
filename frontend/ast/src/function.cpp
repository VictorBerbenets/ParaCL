#include "function.hpp"
#include "visitor.hpp"

#include <iostream>

namespace frontend {

namespace ast {

void scan_function::accept(base_visitor *b_visitor) {
  b_visitor->visit(this);
}

void print_function::accept(base_visitor *b_visitor) {
  b_visitor->visit(this);
}

} // <--- namespace ast

} // <--- namespace frontend
