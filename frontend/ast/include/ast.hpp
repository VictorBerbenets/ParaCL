#pragma once

#include <memory>
#include <utility>
#include <vector>
#include <concepts>

#include "statement.hpp"

namespace frontend {

namespace ast {

template <typename T>
concept derived_from = std::derived_from<T, ast_node>;

class ast final {
  using size_type    = std::size_t;
  using pointer_type = std::unique_ptr<ast_node>;
 public:
  ast() = default;
  ast(const ast&) = delete;
  ast(ast&& other);

  const ast_node *root_ptr() const & noexcept;

  template <derived_from NodeType, typename... Args>
  NodeType *make_node(Args... args) {
    auto node_ptr = std::make_unique<NodeType>(std::forward<Args>(args)...);
    auto ret_ptr  = node_ptr.get();
    nodes_.push_back(std::move(node_ptr));
    size_++;
    return ret_ptr;
  }

  void set_root(ast_node* root_id) & noexcept;

  size_type size() const noexcept;
  [[nodiscard]] bool empty() const noexcept;
 private:
  ast_node *root_ = nullptr;
  size_type size_  = 0;
  std::vector<pointer_type> nodes_;
};

} // <--- namespace ast

} // <--- namespace frontend

