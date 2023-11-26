#include <string>

#include "definition.hpp"



namespace frontend {

namespace ast {

var_definition::var_definition(const std::string& name, expression* expr)
    : definition(name),
      identifier_ {expr} {}
var_definition::var_definition(std::string&& name, expression* expr)
    : definition(std::move(name)),
      identifier_ {expr} {}

} // <--- namespace ast

} // <--- namespace frontend
