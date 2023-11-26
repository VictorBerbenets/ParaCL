#include <utility>
#include <queue>

#include "ast.hpp"

namespace frontend {

namespace ast {

ast::ast(ast&& other)
  : root_ {std::exchange(other.root_, nullptr)},
    size_ {std::exchange(other.size_, 0)},
    nodes_ {std::move(other.nodes_)} {}

const ast_node *ast::root_ptr() const & noexcept { return root_; }

void ast::set_root(ast_node* root_id) & noexcept { root_ = root_id; };

ast::size_type ast::size() const noexcept      { return size_; }
[[nodiscard]] bool ast::empty() const noexcept { return size_ == 0; }

} // <--- namespace ast

} // <--- namespace frontend
