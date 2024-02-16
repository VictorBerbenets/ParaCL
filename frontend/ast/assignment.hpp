#pragma once

#include "expression.hpp"
#include "types.hpp"
#include "statement.hpp"

namespace frontend {

namespace ast {

template <typename T>
class assignment;

template <>
class assignment<int>: public expression {
  using value_type = int;
 public:
  assignment(expression *expr, lvalue_variable *obj, yy::location loc)
    : expression {loc},
      object_ {obj},
      identifier_ {expr} {}

  void accept(base_visitor *base_visitor) override {
    base_visitor->visit(this);
  }

  expression *ident_exp() noexcept {
    return identifier_;
  }

  void redefine(value_type val) {
    object_->set_value(val);
  }

  object_type *get_object() const noexcept {
    return object_;
  }
 private:
  lvalue_variable* object_;
  expression* identifier_;
};

} // <--- namespace ast

} // <--- names
