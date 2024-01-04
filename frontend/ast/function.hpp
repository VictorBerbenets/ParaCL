#pragma once

#include <concepts>
#include <string>
#include <variant>
#include <memory>
#include <optional>

#include "statement.hpp"
#include "expression.hpp"
#include "visitor.hpp"
#include "location.hh"

namespace frontend {

namespace ast {

class function: public statement {
 protected:
  using statement::statement;
/* place for next levels */
};

class print_function: public function {
 public:
  print_function(expression *expr, yy::location loc)
      : function {loc},
        print_expr_ {expr} {}

  void accept(base_visitor *b_visitor) override {
    b_visitor->visit(this);
  }

  expression *get() const noexcept {
    return print_expr_;
  }

 private:
  expression *print_expr_;
};

class scan_function: public function {
  using value_type = int;
 public:
  template <typename VarType = std::string>
  scan_function(statement_block *curr_block, VarType&& var, yy::location loc)
      : var_ {curr_block, std::forward<VarType>(var), loc} {
    var_.declare();
  }

  void accept(base_visitor *b_visitor) override {
    b_visitor->visit(this);
  }

  std::string var_name() {
    return var_.name();
  }
 private:
  variable var_;
};

} // <--- namespace ast

} // <--- namespace frontend
