#pragma once

#include "statement.hpp"
#include "identifiers.hpp"
#include "visitor.hpp"
#include "location.hh"

namespace paracl {

namespace ast {

class expression: public statement {
 protected:
  using statement::statement;

  expression() = default;

  using pointer_type = expression*;
};

class number: public expression {
  using value_type = int;
 public:
  number(value_type num, yy::location loc)
      : expression {loc},
        value_ {num} {}

  const value_type &get_value() const noexcept {
    return value_;
  }

  void accept_interpret(base_visitor *b_visitor) override {
    b_visitor->visit(this);
  }

 private:
  value_type value_;
};

class variable: public expression {
 public:
  variable(statement_block *curr_block, const std::string &var_name,
           yy::location l)
      : expression {curr_block, l},
        name_ {var_name} {}

  variable(statement_block *curr_block, std::string &&var_name,
           yy::location l)
      : expression {curr_block, l},
        name_ {std::move(var_name)} {}

  void declare() {
    scope()->declare(name_);
  }

  void accept_interpret(base_visitor *b_visitor) override {
    b_visitor->visit(this);
  }

  const std::string &name() const noexcept {
    return name_;
  }

 private:
  std::string name_;
};

class un_operator: public expression {
 public:
  un_operator(UnOp type, pointer_type arg, yy::location loc)
      : expression {loc},
        type_ {type},
        arg_ {arg} {}

  void accept_interpret(base_visitor *b_visitor) override {
    b_visitor->visit(this);
  }

  expression *arg() noexcept { return arg_; }
  UnOp type() const noexcept { return type_; }

 private:
  UnOp type_;
  pointer_type arg_;
};

template <typename BinType>
class bin_operator: public expression {
 public:
   bin_operator(BinType type, pointer_type left, pointer_type right,
                yy::location loc)
    : expression {loc},
      type_ {type},
      left_ {left},
      right_ {right} {}

  pointer_type left()  noexcept { return left_;  }
  pointer_type right() noexcept { return right_; }
  BinType type() const noexcept { return type_;  }

 protected:
  BinType type_;
  pointer_type left_, right_;
};

class calc_expression: public bin_operator<CalcOp> {
 public:
  using bin_operator::bin_operator;

  void accept_interpret(base_visitor *b_visitor) override {
    b_visitor->visit(this);
  }
};

class logic_expression: public bin_operator<LogicOp> {
 public:
  using bin_operator::bin_operator;

  void accept_interpret(base_visitor* b_visitor) override {
    b_visitor->visit(this);
  }
};

class assignment: public expression {
 public:
  assignment(statement_block *curr_block, const std::string &name,
             expression *expr, yy::location loc)
    : expression {curr_block, loc},
      name_ {name},
      identifier_ {expr} {
    parent_->declare(name_);
  }

  assignment(statement_block *curr_block, std::string &&name,
             expression *expr, yy::location loc)
      : expression {curr_block, loc},
        name_ {std::move(name)},
        identifier_ {expr} {
    parent_->declare(name_);
  }

  void accept_interpret(base_visitor *base_visitor) override {
    base_visitor->visit(this);
  }

  expression *ident_exp() noexcept {
    return identifier_;
  }

  void redefine(int value) {
    parent_->redefine(name_, value);
  }

  const std::string &name() const noexcept {
    return name_;
  }
 private:
  std::string name_;
  expression* identifier_;
};

class read_expression: public expression {
  using value_type = int;
 public:
  read_expression(yy::location loc): expression {loc} {}

  void accept_interpret(base_visitor *base_visitor) override {
    base_visitor->visit(this);
  }

 private:
  value_type value_;
};

} // <--- namespace ast

} // <--- namespace paracl

