#pragma once

#include <iostream>
#include <vector>
#include <string>

#include "visitor.hpp"
#include "location.hh"

namespace frontend {

class error_handler: public base_visitor {
  using error_type = std::pair<const std::string, yy::location>;
  using size_type  = std::size_t;

 public: 
  void visit(ast::statement_block *stm) override {
    for (auto&& statement : *stm) {
      statement->accept(this);
    }
  }

  void visit(ast::calc_expression *stm) override {
    stm->accept_left(this);
    stm->accept_right(this);
  }

  void visit(ast::logic_expression *stm) override {
    stm->accept_left(this);
    stm->accept_right(this);
  }

  void visit(ast::un_operator *stm) override {
    stm->accept_arg(this);
  }

  void visit(ast::number * /*unused*/) override {}

  void visit(ast::variable *stm) override {
    auto curr_scope = stm->scope();
    if (auto right_scope = curr_scope->find(stm->name()); !right_scope) {
      errors_.push_back({stm->name() + " was not declared in this scope",
                        stm->location()});
    }
  }

  void visit(ast::assignment *stm) override {
    stm->accept_exp(this);
  }

  void visit(ast::if_operator *stm) override {
    stm->accept_condition(this);
    stm->accept_body(this);
  }

  void visit(ast::while_operator *stm) override {
    stm->accept_condition(this);
    stm->accept_body(this);
  }

  void visit(ast::read_expression * /*unused*/) override {}

  void visit(ast::print_function *stm) override {
    auto exp = stm->get();
    exp->accept(this);
  }

  void run(statement_block *root) {
    visit(root);
  }

  void print_errors(std::ostream &stream) const {
    for (auto&& err : errors_) {
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
  std::vector<error_type> errors_;   
};

} // <--- namespace frontend

