#pragma once

#include <concepts>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "statement.hpp"
#include "symbol_table.hpp"

namespace paracl {

namespace ast {

template <typename T>
concept derived_from = std::derived_from<T, statement>;

class ast final {
  using size_type = std::size_t;
  using pointer_type = std::unique_ptr<statement>;

  void swap(ast &rhs) {
    std::swap(root_, rhs.root_);
    std::swap(curr_block_, rhs.curr_block_);
    std::swap(nodes_, rhs.nodes_);
  }

public:
  ast() = default;
  ast(const ast &) = delete;
  ast &operator=(const ast &) = delete;
  ast(ast &&) = default;
  ast &operator=(ast &&) = default;
  ~ast() = default;

  root_statement_block *root_ptr() const & noexcept { return root_; }

  template <derived_from NodeType, typename... Args>
  NodeType *make_node(Args &&...args) {
    auto node_ptr = std::make_unique<NodeType>(std::forward<Args>(args)...);
    auto ret_ptr = node_ptr.get();
    nodes_.push_back(std::move(node_ptr));
    return ret_ptr;
  }

  root_statement_block *make_root_block() {
    curr_block_ = make_node<root_statement_block>(curr_block_);
    return static_cast<root_statement_block *>(curr_block_);
  }

  statement_block *make_block() {
    curr_block_ = make_node<statement_block>(curr_block_);
    return curr_block_;
  }

  void set_root(root_statement_block *root_id) & noexcept { root_ = root_id; }

  void set_curr_block(statement_block *block) & noexcept {
    curr_block_ = block;
  }

  statement_block *get_curr_block() noexcept { return curr_block_; }

  size_type size() const noexcept { return nodes_.size(); }
  [[nodiscard]] bool empty() const noexcept { return nodes_.size() == 0; }

private:
  root_statement_block *root_ = nullptr;
  statement_block *curr_block_ = nullptr;
  std::vector<pointer_type> nodes_;
};

} // namespace ast

} // namespace paracl
