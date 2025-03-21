#pragma once

#include <string>
#include <utility>

#include "expression.hpp"
#include "location.hh"
#include "statement.hpp"
#include "visitor.hpp"

namespace paracl {

namespace ast {

/*-----------------for next levels------------------*/
class definition : public statement {
public:
  definition(statement_block *curr_block, const std::string &name,
             yy::location loc)
      : statement{curr_block, loc}, name_{name} {}
  definition(statement_block *curr_block, std::string &&name, yy::location loc)
      : statement{curr_block, loc}, name_{std::move(name)} {}

  const std::string &name() const noexcept { return name_; }

protected:
  std::string name_;
};
/*-------------------------------------------------*/

} // namespace ast

} // namespace paracl
