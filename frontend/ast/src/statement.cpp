#include "statement.hpp"

namespace frontend {

namespace ast {

void statement::accept(base_visitor* b_visitor) {
  b_visitor->visit(this);
}

} // <--- namespace ast

} // <--- namespace frontend
