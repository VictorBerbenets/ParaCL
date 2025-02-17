#pragma once

#include <concepts>
#include <memory>
#include <vector>
#include <iostream>

#include "visitor.hpp"
#include "codegen_visitor.hpp"
#include "symbol_table.hpp"
#include "location.hh"

namespace paracl {

namespace ast {

class statement;
class statement_block;

template <typename T>
concept module_identifier = std::input_iterator<T> &&
                            std::derived_from<T, statement>;

class statement {
 public:
  virtual ~statement() = default;

  virtual void accept(base_visitor *b_visitor) = 0;
  virtual void accept(CodeGenVisitor *b_visitor) = 0;

  void set_parent(statement_block *parent) noexcept {
    parent_ = parent;
  }

  statement_block *scope() noexcept {
    return parent_;
  }

  yy::location location() const { return loc_; }

 protected:
  explicit statement(yy::location loc)
    : loc_ {loc} {}

  statement(statement_block *parent, yy::location loc = yy::location{})
    : parent_ {parent},
      loc_ {loc} {}

  statement() = default;

  statement_block *parent_;
  yy::location loc_;
};

class statement_block: public statement {
  using StmtsStore     = std::vector<statement*>;
  using ScopeIter      = StmtsStore::iterator;
  using ConstScopeIter = StmtsStore::const_iterator;

  bool has(const std::string &name) const { // find in current scope
    return sym_tab_.has(name);
  }

 public:
  explicit statement_block(statement_block *parent): statement {parent} {}

  statement_block(statement_block *parent, yy::location loc)
      : statement {parent, loc} {}

  statement_block() = default;

  template <module_identifier InputIt>
  statement_block(InputIt begin ,InputIt end)
      : statements_ {begin, end} {}

  void accept(base_visitor *b_visitor) override {
    b_visitor->visit(this);
  }
  
  void accept(CodeGenVisitor *CodeGenVis) override {
    CodeGenVis->visit(this);
  }

  void declare(const std::string &name, int value = 0) {
    if (auto curr_scope = find(name); !curr_scope) {
      sym_tab_.add(name, value);
    }
  }

  void redefine(const std::string &name, int value) {
    if (auto curr_scope = find(name); curr_scope) {
      curr_scope->set(name, value);
    } else {
      std::cout << "error" << std::endl;
    }
  }
  // finding declaration in parent's scopes
  statement_block *find(const std::string &name) {
    for (auto curr_scope = this; curr_scope; curr_scope = curr_scope->parent_) {
      if (curr_scope->has(name)) {
        return curr_scope;
      }
    }
    return nullptr;
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

  unsigned size() const noexcept { return statements_.size(); } 

 private:
  StmtsStore statements_;
  symbol_table sym_tab_;
};

class root_statement_block: public statement_block {
  public:
    using statement_block::statement_block;

     //root_statement_block(statement_block *Parent): statement_block(Parent) {}

    void accept(CodeGenVisitor *CodeGenVis) override {
      CodeGenVis->visit(this);
    }
};

} // <--- namespace ast

} // <--- namespace paracl

