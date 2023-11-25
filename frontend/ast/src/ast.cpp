#include <utility>
#include <queue>

#include "ast.hpp"

namespace frontend {

namespace ast {

ast::ast(const ast& other)
  : size_ {other.size_} {
  if (!other.size_) { return ; }
  /*
    std::queue<statement*> other_nodes; // level tree traversal nodes
    std::queue<statement*> nodes;       // creating nodes

    other_nodes.push(other.root_);
    nodes.push(root_node_);
    while(other_nodes.size()) {
      auto parent     = nodes.front();
      auto other_node = other_nodes.front();
      if (other_node->left_) {
        auto un_ptr = std::make_unique<statement>(*other_node->left_);
        statements_push_
        parent->left_ = 
        other_nodes.push(other_node->left_);
        nodes.push(parent->left_);
      }
      if (other_node->right_) {
        other_nodes.push(other_node->right_);
        nodes.push(parent->right_);
      }
      other_nodes.pop();
      nodes.pop();
    }
    */
}

ast::ast(ast&& other)
  : root_ {std::exchange(other.root_, nullptr)},
    size_ {std::exchange(other.size_, 0)},
    statements_ {std::move(other.statements_)} 
  {}

statement *ast::root_ptr() const & noexcept { return root_; }

void ast::set_root(statement* root_id) & noexcept { root_ = root_id; };
} // <--- namespace ast

} // <--- namespace frontend
