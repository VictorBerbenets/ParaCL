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
  explicit number(int num);
  ~number() override = default;

  const value_type &get_value() const noexcept;
  void accept(base_visitor *b_visitor) override;
 private:
  value_type value_;
};

class variable: public expression {
 public:
  variable(statement_block *curr_block, const std::string &str);
  variable(statement_block *curr_block, std::string &&str);
  ~variable() override = default;

  void accept(base_visitor *b_visitor) override;
  std::string name() noexcept;
 private:
  std::string name_;
};

class un_operator: public expression {
 public:
  un_operator(UnOp type, pointer_type child);
  ~un_operator() override = default;

  void accept(base_visitor *b_visitor) override;
  expression *arg();
  UnOp type_;
 private:
  pointer_type child_;
};

template <typename BinType>
class bin_operator: public expression {
 public:
  ~bin_operator() override = default;

  pointer_type left()  noexcept { return left_;  }
  pointer_type right() noexcept { return right_; }
  BinType type() const noexcept { return type_; }
 protected:
  bin_operator(BinType type, pointer_type left, pointer_type right)
      : type_ {type},
        left_ {left},
        right_ {right} {}

  BinType type_;
  pointer_type left_, right_;
};

class calc_expression: public bin_operator<CalcOp> {
 public:
  calc_expression(CalcOp type, pointer_type left, pointer_type right)
      : bin_operator(type, left, right) {}

  ~calc_expression() override = default;

  void accept(base_visitor *b_visitor) override { b_visitor->visit(this); }
};

class logic_expression: public bin_operator<LogicOp> {
 public:
  logic_expression(LogicOp type, pointer_type left, pointer_type right)
      : bin_operator(type, left, right) {}

  ~logic_expression() override = default;

  void accept(base_visitor* b_visitor) override { b_visitor->visit(this); }
};

} // <--- namespace ast

} // <--- namespace frontend
