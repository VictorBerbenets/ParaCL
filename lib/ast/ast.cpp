#include <cassert>

#include "ast.hpp"

namespace paracl {

namespace ast {

void ast::swap(ast &rhs) {
  std::swap(root_, rhs.root_);
  std::swap(curr_block_, rhs.curr_block_);
  std::swap(nodes_, rhs.nodes_);
}

root_statement_block *ast::make_root_block() {
  curr_block_ = make_node<root_statement_block>(curr_block_);
  return static_cast<root_statement_block *>(curr_block_);
}

statement_block *ast::make_block() {
  curr_block_ = make_node<statement_block>(curr_block_);
  return curr_block_;
}

root_statement_block *ast::root_ptr() const & noexcept { return root_; }

void ast::set_root(root_statement_block *root_id) & noexcept {
  assert(root_id);
  root_ = root_id;
}

void ast::set_curr_block(statement_block *block) & noexcept {
  assert(block);
  curr_block_ = block;
}

statement_block *ast::get_curr_block() noexcept { return curr_block_; }

ast::size_type ast::size() const noexcept { return nodes_.size(); }
[[nodiscard]] bool ast::empty() const noexcept { return nodes_.size() == 0; }

} // namespace ast

} // namespace paracl
