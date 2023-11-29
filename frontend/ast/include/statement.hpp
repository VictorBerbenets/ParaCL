#pragma once

#include <concepts>
#include <memory>
#include <list>

#include "visitor.hpp"

namespace frontend {

namespace ast {

class statement;

template <typename T>
concept module_identifier = std::input_iterator<T> &&
                            std::derived_from<T, statement>;

class i_node {
 public:
  virtual ~i_node() = default;
  virtual void accept(base_visitor* b_visitor) = 0;
};


class statement: public i_node {
 public:
  ~statement() override = default;

  void accept(base_visitor *b_visitor) override;

 private:
  statement* parent_;
};

class statement_block: public statement {
  using stmts_store = std::list<statement*>;
 public:
  explicit statement_block(statement_block* parent);

  template <module_identifier InputIt>
  statement_block(InputIt begin ,InputIt end)
      : statements_ {begin, end} {}

  ~statement_block() override {};

  void accept(base_visitor *b_visitor) override;

  void add(statement *stm) {
    statements_.push_back(stm);
  }
  stmts_store &statements();
 private:
  stmts_store statements_;
};

} // <--- namespace ast

} // <--- namespace frontend
