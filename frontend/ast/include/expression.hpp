#pragma once

#include "statement.hpp"
#include "identifiers.hpp"
#include "visitor.hpp"

namespace frontend {

namespace ast {

class expression: public statement {
 public:
 expression() = default;
   expression(statement_block *curr_block): statement(curr_block) {}
  ~expression() override = default;

  virtual int eval() const = 0;

 protected:
  using pointer_type = expression*;

};

class logic_expression: public expression {
 public:
  logic_expression(LogicOp type, expression *left, expression *right);
  ~logic_expression() override = default;

  int eval() const override;
  expression* left() const noexcept;
  expression* right() const noexcept;
  void accept(base_visitor* b_visitor) override;

  LogicOp type_;
 private:
  expression *left_, *right_;
};

class number: public expression {
  using value_type = int;
 public:
  number(int num);
  ~number() override = default;

  value_type eval() const;
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

  int eval() const;
  void accept(base_visitor *b_visitor) override;
  std::string name() noexcept;
  statement_block *scope() noexcept;
 private:
  std::string name_;
};

class bin_operator: public expression {
 public:
  bin_operator(BinOp type, pointer_type left, pointer_type right);
  ~bin_operator() override = default;

  int eval() const override;
  void accept(base_visitor *b_visitor) override;
  expression* left() const noexcept;
  expression* right() const noexcept;
  BinOp type_;
 private:
  pointer_type left_, right_;
};

class un_operator: public expression {
 public:
  un_operator(UnOp type, pointer_type child);
  ~un_operator() override = default;

  int eval() const override;
  void accept(base_visitor *b_visitor) override;
  expression *arg();
  UnOp type_;
 private:
  pointer_type child_;
};


} // <--- namespace ast

} // <--- namespace frontend
