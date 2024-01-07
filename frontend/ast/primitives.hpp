#pragma once

#include "statement.hpp"
#include "location.hh"

namespace frontend {

namespace ast {

class scolon: public statement {
 public:
  scolon(yy::location loc): statement{loc} {}

  void accept(base_visitor* /*unused*/) override {}
};

} // <--- namespace ast

} // <--- namespace frontend
