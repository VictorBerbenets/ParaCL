#include <utility>
#include <queue>

#include "ast.hpp"

namespace frontend {

namespace ast {

ast::ast(ast&& other)
  : root_ {std::exchange(other.root_, nullptr)},
    size_ {std::exchange(other.size_, 0)},
    statements_ {std::move(other.statements_)} 
  {}

statement *ast::root_ptr() const & noexcept { return root_; }

void ast::set_root(statement* root_id) & noexcept { root_ = root_id; };

} // <--- namespace ast

} // <--- namespace frontend
