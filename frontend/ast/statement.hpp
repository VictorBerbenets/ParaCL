#pragma once

#include <concepts>
#include <memory>
#include <vector>
#include <iostream>

#include "visitor.hpp"
#include "symbol_table.hpp"

namespace frontend {

namespace ast {

class statement;

template <typename T>
concept module_identifier = std::input_iterator<T> &&
                            std::derived_from<T, statement>;

class statement_block;

class statement {
 public:
  virtual ~statement() = default;

  virtual void accept(base_visitor *b_visitor) = 0;

  void set_parent(statement_block *parent) noexcept {
    parent_ = parent;
  }

  statement_block *scope() noexcept {
    return parent_;
  }

 protected:
  explicit statement(statement_block *parent) noexcept
      : parent_ {parent} {}

  statement() = default;

  statement_block *parent_;
};

class statement_block final: public statement {
  using StmtsStore     = std::vector<statement*>;
  using ScopeIter      = StmtsStore::iterator;
  using ConstScopeIter = StmtsStore::const_iterator;
 public:
  explicit statement_block(statement_block *parent)
      : statement {parent} {}

  statement_block() = default;

  template <module_identifier InputIt>
  statement_block(InputIt begin ,InputIt end)
      : statements_ {begin, end} {}

  ~statement_block() override {};

  void accept(base_visitor *b_visitor) override {
    b_visitor->visit(this);
  }

  // template <typename... Args>
  void declare(const std::string &name, int value = 0) {
    for (auto curr_scope = this; curr_scope; curr_scope = curr_scope->parent_) {
      if (curr_scope->has(name)) {
        return ;
      }
    }
    sym_tab_.add(name, value);
  }

  void redefine(const std::string &name, int value) {
    for (auto curr_scope = this; curr_scope; curr_scope = curr_scope->parent_) {
      if (curr_scope->has(name)) {
        curr_scope->set(name, value);
        return ;
      }
    }
    std::cout << "error" << std::endl;
  }

  bool has(const std::string &name) const {
    return sym_tab_.has(name);
  }

  void set(const std::string &name, int value) {
    sym_tab_[name] = value;
  }

  int value(const std::string &name) noexcept {
    return sym_tab_[name];
  }

  void add(statement *stm) {
    stm->set_parent(this);
    statements_.push_back(stm);
  }

  ScopeIter begin() noexcept { return statements_.begin(); }
  ScopeIter end()   noexcept { return statements_.end(); }
  ConstScopeIter cbegin() const noexcept { return statements_.cbegin(); }
  ConstScopeIter cend()   const noexcept { return statements_.cend(); }

 private:
  StmtsStore statements_;
  symbol_table sym_tab_;
};

} // <--- namespace ast

} // <--- namespace frontend
