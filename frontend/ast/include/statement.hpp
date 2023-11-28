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
  virtual void accept(visitor* visitor) = 0;
};


class statement: public i_node {
 public:
  ~statement() override = default;
  void accept(visitor* visitor) override {

  }

 private:
  statement* parent_;
};

class statement_block: public statement {
 public:
  statement_block() = default;

  template <module_identifier InputIt>
  statement_block(InputIt begin ,InputIt end)
      : statements_ {begin, end} {}

  ~statement_block() override {};

  void add(statement* stm) {
    statements_.push_back(stm);
  }
 private:
  std::list<statement*> statements_;
};

} // <--- namespace ast

} // <--- namespace frontend
