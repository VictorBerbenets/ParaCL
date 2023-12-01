#include <utility>
#include <queue>

#include "ast.hpp"
#include "statement.hpp"

namespace frontend {

namespace ast {

ast::ast(ast&& other)
  : root_ {std::exchange(other.root_, nullptr)},
    size_ {std::exchange(other.size_, 0)},
    nodes_ {std::move(other.nodes_)} {}

statement_block *ast::root_ptr() const & noexcept { return root_; }

void ast::set_root(statement_block *root_id) & noexcept { root_ = root_id; };

void ast::set_curr_block(statement_block *block) & noexcept {
  curr_block_ = block;
}

statement_block * ast::get_curr_block() noexcept {
  return curr_block_;
}

statement_block *ast::make_block() {
  return curr_block_ = make_node<statement_block>(curr_block_);
}

ast::size_type ast::size() const noexcept      { return size_; }
[[nodiscard]] bool ast::empty() const noexcept { return size_ == 0; }

} // <--- namespace ast

} // <--- namespace frontend
