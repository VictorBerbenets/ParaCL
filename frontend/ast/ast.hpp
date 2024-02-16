#pragma once

#include <memory>
#include <utility>
#include <vector>
#include <concepts>
#include <memory>

#include "statement.hpp"
#include "statement_block.hpp"
#include "symbol_table.hpp"
#include "types.hpp"
#include "base_type.hpp"
#include "assignment.hpp"

namespace yy {

class driver;

}

namespace frontend {

namespace ast {

template <typename T, typename From = statement>
concept derived_from = std::derived_from<T, From>;

class ast final {
  using size_type    = std::size_t;
  using pointer_type = std::unique_ptr<statement>;

  void swap(ast &rhs) {
    std::swap(root_, rhs.root_);
    std::swap(curr_block_, rhs.curr_block_);
    std::swap(nodes_, rhs.nodes_);
  }

 public:
  ast() = default;
  ast(const ast& ) = delete;
  ast &operator=(const ast&) = delete;
  ast(ast&& ) = default;
  ast &operator=(ast&& ) = default;
  ~ast() = default;

  statement_block *root_ptr() const & noexcept { return root_; }

  template <derived_from NodeType, typename... Args>
  requires (!std::derived_from<NodeType, object_type>)
  NodeType *make_node(Args&&... args) {
    auto node_ptr = std::make_unique<NodeType>(std::forward<Args>(args)...);
    auto ret_ptr  = node_ptr.get();
    nodes_.push_back(std::move(node_ptr));
    return ret_ptr;
  }

  template <derived_from<object_type> NodeType, typename... Args>
  NodeType *make_node(std::string obj_name, Args&&... args) {
    if (auto right_block = curr_block_->find(obj_name); right_block) {
      return static_cast<NodeType*>(right_block->object(obj_name));
    }
    auto node_ptr = std::make_unique<NodeType>(std::move(obj_name), std::forward<Args>(args)...);
    auto ret_ptr  = node_ptr.get();
    nodes_.push_back(std::move(node_ptr));
    return ret_ptr;
  }
  
  template <typename T>
  assignment<int> *make_node(std::string obj_name, statement_block *curr_scope,
                             expression *right_oper, yy::location loc) {
    if (auto right_block = curr_block_->find(obj_name); right_block) {
      auto object     = static_cast<T*>(right_block->object(obj_name));
      auto assign_ptr = make_node<assignment<int>>(right_oper, object, loc);
      return assign_ptr;
    }
    auto obj_type   = std::make_unique<T>(obj_name, curr_scope, loc);
    auto assign_ptr = make_node<assignment<int>>(right_oper, obj_type.get(), loc);
    curr_block_->declare(obj_type.get());
    nodes_.push_back(std::move(obj_type));
    return assign_ptr;
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

  size_type size() const noexcept { return nodes_.size(); }
  [[nodiscard]] bool empty() const noexcept { return nodes_.size() == 0; }

  friend class ::yy::driver;
 private:
  statement_block *root_       = nullptr;
  statement_block *curr_block_ = nullptr;
  std::vector<pointer_type> nodes_;
};

} // <--- namespace ast

} // <--- namespace frontend

