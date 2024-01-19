#pragma once

#include <memory>
#include <utility>
#include <vector>
#include <concepts>
#include <memory>

#include "statement.hpp"
#include "symbol_table.hpp"

namespace frontend {

namespace ast {

template <typename T>
concept derived_from = std::derived_from<T, statement>;

class ast final {
  using size_type    = std::size_t;
  using pointer_type = std::unique_ptr<statement>;

  void swap(ast &rhs) {
    std::swap(size_, rhs.size_);  
    std::swap(root_, rhs.root_);  
    std::swap(curr_block_, rhs.curr_block_);  
    std::swap(nodes_, rhs.nodes_);  
  }

 public:
  ast() = default;
  ast(const ast &rhs): size_ {rhs.size_} { /*TODO*/ }
  
  ast &operator=(const ast &rhs) {
    if (this == std::addressof(rhs)) {
      return *this;
    }
    auto tmp = rhs;
    swap(tmp);
    return *this;
  }

  ast(ast&& ) = default;
  ast &operator=(ast&& ) = default;
  ~ast() = default;

  statement_block *root_ptr() const & noexcept { return root_; }

  template <derived_from NodeType, typename... Args>
  NodeType *make_node(Args&&... args) {
    auto node_ptr = std::make_unique<NodeType>(std::forward<Args>(args)...);
    auto ret_ptr  = node_ptr.get();
    nodes_.push_back(std::move(node_ptr));
    size_++;
    return ret_ptr;
  }

  statement_block *make_block() {
    return curr_block_ = make_node<statement_block>(curr_block_);
  }

  void set_root(statement_block *root_id) & noexcept {
    root_ = root_id;
  }

  void set_curr_block(statement_block *block) & noexcept {
    curr_block_ = block;
  }

  statement_block *get_curr_block() noexcept { return curr_block_; }

  size_type size() const noexcept { return size_; }
  [[nodiscard]] bool empty() const noexcept { return size_ == 0; }
 private:
  statement_block *root_       = nullptr;
  statement_block *curr_block_ = nullptr;
  std::vector<pointer_type> nodes_;
  size_type size_ = 0;
};

} // <--- namespace ast

} // <--- namespace frontend

