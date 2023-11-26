#pragma once

#include "statement.hpp"
#include "expression.hpp"

namespace frontend {

namespace ast {

class function: public statement {

};

class print_function: public function {

};

class scan_function: public function {

};

} // <--- namespace ast

} // <--- namespace frontend
