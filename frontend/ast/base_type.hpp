#pragma once

#include <vector>
#include <string>

#include "expression.hpp"
#include "location.hh"

namespace frontend {

namespace ast {

class object_type: public expression {
 public:
  using size_type = std::size_t;

  object_type() = default; // remove in future

  object_type(std::string name, statement_block *block, yy::location loc)
      : expression {block, loc},
        name_ {std::move(name)} {}

  const std::string &name() const noexcept {
    return name_;
  }

 protected:
  std::string name_;
};
 
} // <--- namespace ast

} // <--- namespace frontend

