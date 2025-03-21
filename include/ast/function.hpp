#pragma once

#include <concepts>
#include <memory>
#include <optional>
#include <string>
#include <variant>

#include "expression.hpp"
#include "location.hh"
#include "statement.hpp"

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

  void accept(VisitorBase *Vis) override { Vis->visit(this); }

  expression *get() const noexcept { return print_expr_; }

private:
  expression *print_expr_;
};

} // namespace ast

} // namespace paracl
