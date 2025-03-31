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

class print_function : public expression {
public:
  print_function(expression *expr, yy::location loc)
      : expression{loc}, print_expr_{expr} {}

  ResultValue accept(VisitorBasePtr Vis) override { return Vis->visit(this); }

  expression *get() const noexcept { return print_expr_; }

private:
  expression *print_expr_;
};

} // namespace ast
} // namespace paracl
