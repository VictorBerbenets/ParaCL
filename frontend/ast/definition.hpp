#pragma once

#include <string>
#include <utility>

#include "statement.hpp"
#include "expression.hpp"
#include "visitor.hpp"

namespace frontend {

namespace ast {

/*-----------------for next levels------------------*/
class definition: public statement {
 public:
  definition(statement_block *curr_block, const std::string &name)
      : statement(curr_block), name_ {name} {
    parent_->declare(name_);
  }
  definition(statement_block *curr_block, std::string &&name)
      : statement(curr_block), name_ {std::move(name)} {
    parent_->declare(name_);
  }

  ~definition() override = default;

  const std::string &name() const noexcept {
    return name_;
  }

 protected:
  std::string name_;
};
/*-------------------------------------------------*/

} // <--- namespace ast

} // <--- namespace frontend
