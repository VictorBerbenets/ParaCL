#pragma once

#include <concepts>
#include <memory>
#include <list>

// #include "visitor"

namespace frontend {

namespace ast {

class i_node {
 public:
  virtual ~i_node() = default;
};

template <typename T>
concept module_identifier = std::input_iterator<T> &&
                            std::derived_from<T, i_node>;

class statement: public i_node {
 public:
  //virtual void accept(visitor* visitor) = 0;
  virtual ~statement() = default;

 private:
  statement* parent_;
};

class statement_block: public statement {
 public:
  statement_block() = default;

  template <module_identifier InputIt>
  statement_block(InputIt begin ,InputIt end)
      : statements_ {begin, end} {}

  virtual ~statement_block() = default;

  void add(statement* stm) {
    statements_.push_back(stm);
  }
 private:
  std::list<statement*> statements_;
};

} // <--- namespace ast

} // <--- namespace frontend
