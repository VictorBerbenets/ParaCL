#pragma once

#include <concepts>
#include <memory>
#include <optional>
#include <string>
#include <variant>

#include "expression.hpp"
#include "location.hh"
#include "statement.hpp"
#include "visitor.hpp"

namespace paracl {

namespace ast {

class function : public statement {
protected:
  using statement::statement;
  /* place for next levels */
};

class print_function : public function {
public:
  print_function(expression *expr, yy::location loc)
      : function{loc}, print_expr_{expr} {}

  void accept(base_visitor *b_visitor) override { b_visitor->visit(this); }

  void accept(CodeGenVisitor *CodeGenVis) override { CodeGenVis->visit(this); }

  expression *get() const noexcept { return print_expr_; }

private:
  expression *print_expr_;
};

} // namespace ast

} // namespace paracl
