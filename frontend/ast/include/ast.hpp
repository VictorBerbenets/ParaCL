#pragma once

#include <memory>
#include <utility>
#include <vector>
#include <concepts>

#include "statement.hpp"

namespace frontend {

namespace ast {

template <typename T>
concept derived_from = std::derived_from<T, statement>;

class ast final {
    using size_type    = std::size_t;
    using pointer_type = std::unique_ptr<statement>;
  public:
    ast() = default;
    ast(const ast&) = delete;
    ast(ast&& other);

    statement *root_ptr() const & noexcept;

    template <derived_from NodeType, typename... Args>
    NodeType *make_node(Args... args) {
      auto node_ptr = std::make_unique<NodeType>(std::forward<Args>(args)...);
      auto ret_ptr  = node_ptr.get();
      statements_.push_back(std::move(node_ptr));

      return ret_ptr;
    }
    
    void set_root(statement* root_id) & noexcept;
  private:
    statement *root_ = nullptr;
    size_type size_  = 0;
    std::vector<pointer_type> statements_;
};

} // <--- namespace ast

} // <--- namespace frontend

