#pragma once

#include "llvm/Support/FormatVariadic.h"

#include <iostream>
#include <vector>

#include "ast_includes.hpp"
#include "location.hh"
#include "visitor_tracker.hpp"

namespace paracl {

class ErrorHandler : public VisitorTracker {
  using ErrorType = std::pair<const std::string, yy::location>;

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
    auto curr_scope = Var->scope();
    std::cout << "LINE = " << __LINE__ << std::endl;
    llvm::outs() << "NAME = " << Var->name() << '\n';
    if (!SymTbl.isDefined({Var->name(), curr_scope})) {
      std::cout << "LINE = " << __LINE__ << std::endl;
      Errors.push_back(
          {llvm::formatv("{0} was not declared in this scope", Var->name()), Var->location()});
    }
  }

  void visit(ast::assignment *Assign) override {
    std::cout << "LINE = " << __LINE__ << std::endl;
    auto *IdentExp = getValueAfterAccept(Assign->getIdentExp());
    if (ValManager.getValueFor({Assign->name(), Assign->scope()})) {
    std::cout << "LINE = " << __LINE__ << std::endl;
      auto *LValue = getValueAfterAccept(Assign->getLValue());
    std::cout << "LINE = " << __LINE__ << std::endl;
      assert(LValue);
      assert(IdentExp);
      if (LValue->getType() != IdentExp->getType()) {
      }
    }
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
    for (auto &&err : Errors) {
      stream << err.second << " : " << err.first << std::endl;
    }
  }

  [[nodiscard]] bool empty() const noexcept { return Errors.empty(); }
  unsigned size() const noexcept { return Errors.size(); }

  auto begin() noexcept { return Errors.begin(); }
  auto end() noexcept { return Errors.end(); }
  auto cbegin() const noexcept { return Errors.cbegin(); }
  auto cend() const noexcept { return Errors.cend(); }

private:
  SymTable &SymTbl;
  ValueManager &ValManager;
  std::vector<ErrorType> Errors;
};

} // namespace paracl
