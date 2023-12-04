#pragma once

#include <string>
#include <utility>

#include "statement.hpp"
#include "expression.hpp"
#include "visitor.hpp"

namespace frontend {

namespace ast {

class definition: public statement {
 public:
  definition(statement_block *curr_block, const std::string &name)
      : statement(curr_block), name_ {name} {
    parent_->declare(name_);
  }
  definition(statement_block *curr_block, std::string &&name)
      : statement(curr_block), name_ {std::move(name)} {
    parent_->declare(name_);
  }

  ~definition() override = default;

  const std::string &name() const noexcept {
    return name_;
  }

 protected:
  std::string name_;
};

class assignment: public definition {
 public:
    assignment(statement_block *curr_block, const std::string &name, expression *expr)
        : definition {curr_block, name},
          identifier_ {expr} {}

    assignment(statement_block *curr_block, std::string &&name, expression *expr)
        : definition {curr_block, std::move(name)},
          identifier_ {expr} {}

    ~assignment() override = default;

    void accept(base_visitor *base_visitor) override {
      base_visitor->visit(this);
    }

    void accept_exp(base_visitor *b_visitor) {
      identifier_->accept(b_visitor);
    }

    expression *ident_exp() noexcept {
      return identifier_;
    }

    void redefine(int value) {
      parent_->redefine(name_, value);
    }

 private:
  expression* identifier_;
};

} // <--- namespace ast

} // <--- namespace frontend
