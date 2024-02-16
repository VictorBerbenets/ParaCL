#pragma once

#include <vector>
#include <concepts>

#include "statement.hpp"
#include "symbol_table.hpp"
#include "location.hh"
#include "base_type.hpp"

namespace frontend {

namespace ast {

class integer_variable;

template <typename T>
concept module_identifier = std::input_iterator<T> &&
                            std::derived_from<T, statement>;

class statement_block final: public statement {
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

  void declare(object_type *obj_ptr) {
    if (auto curr_scope = find(obj_ptr->name()); !curr_scope) {
      sym_tab_.add(obj_ptr);
    }
  }
 
  void redefine(object_type *obj_ptr) {
    auto curr_scope = find(obj_ptr->name());
    curr_scope->set(obj_ptr);
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

  const statement_block *find(const std::string &name) const {
    return find(name);
  }

  void set(object_type *obj_ptr) {
    sym_tab_[obj_ptr->name()] = obj_ptr;
  }

  object_type *object(const std::string &name) noexcept {
    return has(name) ? sym_tab_[name] : nullptr;
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

} // <-- namespace ast

} // <--- namespace frontend
