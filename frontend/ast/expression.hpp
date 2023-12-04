#pragma once

#include "statement.hpp"
#include "identifiers.hpp"
#include "visitor.hpp"

namespace frontend {

namespace ast {

class expression: public statement {
 public:
  ~expression() override = default;

 protected:
  expression() = default;
  explicit expression(statement_block *curr_block): statement(curr_block) {}

  using pointer_type = expression*;
};

class number: public expression {
  using value_type = int;
 public:
  explicit number(int num)
      : value_ {num} {}

  ~number() override = default;

  const value_type &get_value() const noexcept {
    return value_;
  }

  void accept(base_visitor *b_visitor) override {
    b_visitor->visit(this);
  }

 private:
  value_type value_;
};

class variable: public expression {
 public:
  variable(statement_block *curr_block, const std::string &var_name)
      : expression {curr_block},
        name_ {var_name} {}

  variable(statement_block *curr_block, std::string &&var_name)
      : expression {curr_block},
        name_ {var_name} {}

  ~variable() override = default;

  void declare() {
    scope()->declare(name_);
  }

  void accept(base_visitor *b_visitor) override {
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
  un_operator(UnOp type, pointer_type arg)
      : type_ {type},
        arg_ {arg} {}

  ~un_operator() override = default;

  void accept(base_visitor *b_visitor) override {
    b_visitor->visit(this);
  }

  expression *arg() noexcept { return arg_; }
  UnOp type() const noexcept { return type_; }

  void accept_arg(base_visitor *b_visitor) {
    arg_->accept(b_visitor);
  }

 private:
  UnOp type_;
  pointer_type arg_;
};

template <typename BinType>
class bin_operator: public expression {
 public:
   bin_operator(BinType type, pointer_type left, pointer_type right)
      : type_ {type},
        left_ {left},
        right_ {right} {}

  ~bin_operator() override = default;

  pointer_type left()  noexcept { return left_;  }
  pointer_type right() noexcept { return right_; }
  BinType type() const noexcept { return type_; }

  void accept_left(base_visitor *b_visitor) {
    left_->accept(b_visitor);
  }

  void accept_right(base_visitor *b_visitor) {
    right_->accept(b_visitor);
  }

 protected:
  BinType type_;
  pointer_type left_, right_;
};

class calc_expression: public bin_operator<CalcOp> {
 public:
  using bin_operator::bin_operator;

  ~calc_expression() override = default;

  void accept(base_visitor *b_visitor) override {
    b_visitor->visit(this);
  }
};

class logic_expression: public bin_operator<LogicOp> {
 public:
  using bin_operator::bin_operator;

  ~logic_expression() override = default;

  void accept(base_visitor* b_visitor) override {
    b_visitor->visit(this);
  }
};

} // <--- namespace ast

} // <--- namespace frontend
