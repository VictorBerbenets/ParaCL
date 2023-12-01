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

class i_node {
 public:
  virtual ~i_node() = default;
  virtual void accept(base_visitor *b_visitor) = 0;
};

class statement_block;

class statement: public i_node {
 public:
  ~statement() override = default;
  statement(statement_block *parent) noexcept;
  statement() = default;

  void accept(base_visitor *b_visitor) override;
  void set_parent(statement_block *parent) noexcept;
  statement_block *scope() noexcept { return parent_; }
 protected:
  statement_block *parent_;
};

class statement_block: public statement {
  using stmts_store = std::vector<statement*>;
 public:
  explicit statement_block(statement_block *parent);
  statement_block() = default;

  template <module_identifier InputIt>
  statement_block(InputIt begin ,InputIt end)
      : statements_ {begin, end} {}

  ~statement_block() override {};

  void accept(base_visitor *b_visitor) override;

  // template <typename... Args>
  void declare(const std::string &name, int value = 0) {
    if (!sym_tab_.has(name)) {
      sym_tab_.add(name, value);
      std::cout << "ADDED VAR WITH NAME = " << name << std::endl;
      //throw std::logic_error{"Multiple definition of the: " + name};
    }
  }

  void redefine(const std::string &name, int value) {
    if (sym_tab_.has(name)) {
      sym_tab_[name] = value;
    } else {
      std::cout << "error" << std::endl;
    }
  }

  bool has(const std::string &name) const {
    return sym_tab_.has(name);
  }

  int value(const std::string &name) noexcept {
    return sym_tab_[name];
  }

  void add(statement *stm);
  stmts_store &statements();
 private:
  stmts_store statements_;
  symbol_table sym_tab_;
};

} // <--- namespace ast

} // <--- namespace frontend
