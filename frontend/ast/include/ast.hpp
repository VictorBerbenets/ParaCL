#pragma once

#include <memory>
#include <utility>
#include <vector>
#include <concepts>

#include "statement.hpp"
#include "symbol_table.hpp"

namespace frontend {

namespace ast {

template <typename T>
concept derived_from = std::derived_from<T, i_node>;

class ast final {
  using size_type    = std::size_t;
  using pointer_type = std::unique_ptr<i_node>;
 public:
  ast() = default;
  ast(const ast&) = delete;
  ast(ast&& other);

  statement_block *root_ptr() const & noexcept;

  template <derived_from NodeType, typename... Args>
  NodeType *make_node(Args&&... args) {
    auto node_ptr = std::make_unique<NodeType>(std::forward<Args>(args)...);
    auto ret_ptr  = node_ptr.get();
    nodes_.push_back(std::move(node_ptr));
    size_++;
    return ret_ptr;
  }

  statement_block *make_block();

  void set_root(statement_block *root_id) & noexcept;
  void set_curr_block(statement_block *block) & noexcept;
  statement_block *get_curr_block() noexcept;

  size_type size() const noexcept;
  [[nodiscard]] bool empty() const noexcept;

 private:
  statement_block *root_       = nullptr;
  statement_block *curr_block_ = nullptr;
  std::vector<pointer_type> nodes_;
  size_type size_ = 0;
};

} // <--- namespace ast

} // <--- namespace frontend

