#pragma once

#include "llvm/Support/FormatVariadic.h"

#include <iostream>
#include <vector>

#include "ast_includes.hpp"
#include "location.hh"
#include "visitor.hpp"

namespace paracl {

class ErrorHandler : public VisitorBase {
  using error_type = std::pair<const std::string, yy::location>;
  using size_type = std::size_t;

public:
  void visit(ast::ArrayAccessAssignment *Arr) override {}
  void visit(ast::PresetArray *InitListArr) override {}
  void visit(ast::ArrayAccess *ArrAccess) override {}
  void visit(ast::UniformArray *Arr) override {}

  ErrorHandler(SymTable &SymTbl, ValueManager &ValManager)
      : SymTbl(SymTbl), ValManager(ValManager) {}

  void visit(ast::statement_block *stm) override {
    for (auto &&statement : *stm) {
      statement->accept(this);
    }
  }

  void visit(ast::calc_expression *stm) override {
    stm->left()->accept(this);
    stm->right()->accept(this);
  }

  void visit(ast::logic_expression *stm) override {
    stm->left()->accept(this);
    stm->right()->accept(this);
  }

  void visit(ast::un_operator *stm) override { stm->arg()->accept(this); }

  void visit(ast::number * /*unused*/) override {}

  void visit(ast::variable *Var) override {
#if 0
    auto curr_scope = Var->scope();
    if (!SymTbl.isDefined({Var->name(), curr_scope}))
      errors_.push_back(
          {llvm::formatv("{0} was not declared in this scope", Var->name()), Var->location()});
#endif
  }

  void visit(ast::assignment *stm) override {
    stm->getIdentExp()->accept(this);
  }

  void visit(ast::if_operator *stm) override {
    stm->condition()->accept(this);
    stm->body()->accept(this);
  }

  void visit(ast::while_operator *stm) override {
    stm->condition()->accept(this);
    stm->body()->accept(this);
  }

  void visit(ast::read_expression * /*unused*/) override {}

  void visit(ast::print_function *stm) override {
    auto exp = stm->get();
    exp->accept(this);
  }

  void run(ast::statement_block *root) { visit(root); }

  void print_errors(std::ostream &stream) const {
    for (auto &&err : errors_) {
      stream << err.second << " : " << err.first << std::endl;
    }
  }

  [[nodiscard]] bool empty() const noexcept { return errors_.empty(); }
  size_type size() const noexcept { return errors_.size(); }

  auto begin() noexcept { return errors_.begin(); }
  auto end() noexcept { return errors_.end(); }
  auto cbegin() const noexcept { return errors_.cbegin(); }
  auto cend() const noexcept { return errors_.cend(); }

private:
  SymTable &SymTbl;
  ValueManager &ValManager;
  std::vector<error_type> errors_;
};

} // namespace paracl
