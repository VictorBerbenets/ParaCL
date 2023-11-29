#include <string>

#include "definition.hpp"
#include "visitor.hpp"

namespace frontend {

namespace ast {

var_definition::var_definition(const std::string &name, expression *expr)
    : definition(name),
      identifier_ {expr} {}
var_definition::var_definition(std::string &&name, expression *expr)
    : definition(std::move(name)),
      identifier_ {expr} {}

void var_definition::accept(base_visitor *b_visitor) {
  b_visitor->visit(this);
}

} // <--- namespace ast

} // <--- namespace frontend
