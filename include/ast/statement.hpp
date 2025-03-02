#pragma once

#include <concepts>
#include <vector>

#include "codegen_visitor.hpp"
#include "location.hh"
#include "visitor.hpp"

namespace paracl {

namespace ast {

class statement;
class statement_block;

template <typename T>
concept module_identifier =
    std::input_iterator<T> && std::derived_from<T, statement>;

class statement {
public:
  virtual ~statement() = default;

  virtual void accept(base_visitor *b_visitor) = 0;
  virtual void accept(CodeGenVisitor *b_visitor) = 0;

  void set_parent(statement_block *parent) noexcept { parent_ = parent; }

  statement_block *scope() noexcept { return parent_; }

  yy::location location() const { return loc_; }

protected:
  explicit statement(yy::location loc) : loc_{loc} {}

  statement(statement_block *parent, yy::location loc = yy::location{})
      : parent_{parent}, loc_{loc} {}

  statement() = default;

  statement_block *parent_;
  yy::location loc_;
};

class statement_block : public statement {
  using StmtsStore = std::vector<statement *>;

public:
  using ScopeIter = StmtsStore::iterator;
  using ConstScopeIter = StmtsStore::const_iterator;

  explicit statement_block(statement_block *parent) : statement{parent} {}

  statement_block(statement_block *parent, yy::location loc)
      : statement{parent, loc} {}

  statement_block() = default;

  template <module_identifier InputIt>
  statement_block(InputIt begin, InputIt end) : statements_{begin, end} {}

  void accept(base_visitor *b_visitor) override { b_visitor->visit(this); }

  void accept(CodeGenVisitor *CodeGenVis) override { CodeGenVis->visit(this); }

  void add(statement *stm) {
    stm->set_parent(this);
    statements_.push_back(stm);
  }

  ScopeIter begin() noexcept { return statements_.begin(); }
  ScopeIter end() noexcept { return statements_.end(); }
  ConstScopeIter cbegin() const noexcept { return statements_.cbegin(); }
  ConstScopeIter cend() const noexcept { return statements_.cend(); }

  unsigned size() const noexcept { return statements_.size(); }

private:
  StmtsStore statements_;
};

class root_statement_block : public statement_block {
public:
  using statement_block::statement_block;

  void accept(base_visitor *b_visitor) override { b_visitor->visit(this); }

  void accept(CodeGenVisitor *CodeGenVis) override { CodeGenVis->visit(this); }
};

} // namespace ast

} // namespace paracl
