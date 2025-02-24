#pragma once

#include <concepts>
#include <memory>
#include <utility>
#include <vector>

#include "statement.hpp"

namespace paracl {

namespace ast {

template <typename T>
concept derived_from = std::derived_from<T, statement>;

class ast final {
  using size_type = std::size_t;
  using pointer_type = std::unique_ptr<statement>;

  void swap(ast &rhs);

public:
  ast() = default;
  ast(const ast &) = delete;
  ast &operator=(const ast &) = delete;
  ast(ast &&) = default;
  ast &operator=(ast &&) = default;
  ~ast() = default;

  root_statement_block *root_ptr() const & noexcept;

  template <derived_from NodeType, typename... Args>
  NodeType *make_node(Args &&...args) {
    auto node_ptr = std::make_unique<NodeType>(std::forward<Args>(args)...);
    auto ret_ptr = node_ptr.get();
    nodes_.push_back(std::move(node_ptr));
    return ret_ptr;
  }

  root_statement_block *make_root_block();

  statement_block *make_block();

  void set_root(root_statement_block *root_id) & noexcept;

  void set_curr_block(statement_block *block) & noexcept;

  statement_block *get_curr_block() noexcept;

  size_type size() const noexcept;
  [[nodiscard]] bool empty() const noexcept;

private:
  root_statement_block *root_ = nullptr;
  statement_block *curr_block_ = nullptr;
  std::vector<pointer_type> nodes_;
};

} // namespace ast

} // namespace paracl
