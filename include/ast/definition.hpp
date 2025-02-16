#pragma once

#include <string>
#include <utility>

#include "statement.hpp"
#include "expression.hpp"
#include "visitor.hpp"
#include "location.hh"

namespace paracl {

namespace ast {

/*-----------------for next levels------------------*/
class definition: public statement {
 public:
  definition(statement_block *curr_block, const std::string &name,
             yy::location loc)
      : statement{curr_block, loc},
        name_ {name} {
    parent_->declare(name_);
  }
  definition(statement_block *curr_block, std::string &&name,
             yy::location loc)
      : statement{curr_block, loc},
        name_ {std::move(name)} {
    parent_->declare(name_);
  }

  const std::string &name() const noexcept {
    return name_;
  }

 protected:
  std::string name_;
};
/*-------------------------------------------------*/

} // <--- namespace ast

} // <--- namespace paracl
