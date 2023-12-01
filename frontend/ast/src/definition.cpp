#include <string>
#include <iostream>

#include "definition.hpp"
#include "visitor.hpp"

namespace frontend {

namespace ast {

assignment::assignment(statement_block *curr_block,
                               const std::string &name,
                               expression *expr)
    : definition(curr_block, name),
      identifier_ {expr} {
  parent_->declare(name_);
}

assignment::assignment(statement_block *curr_block,
                               std::string &&name,
                               expression *expr)
    : definition(curr_block, std::move(name)),
      identifier_ {expr} {
  parent_->declare(name_);
}

void assignment::accept(base_visitor *b_visitor) {
  b_visitor->visit(this);
}

} // <--- namespace ast

} // <--- namespace frontend
