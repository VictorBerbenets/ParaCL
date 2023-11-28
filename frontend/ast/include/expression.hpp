#pragma once

#include "statement.hpp"
#include "identifiers.hpp"

namespace frontend {

namespace ast {

// expr nodes
class expression: public statement {
 public:
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
 private:
  LogicOp type_;
  expression *left_, *right_;

};

class number: public expression {
  using value_type = int;
 public:
  number(int num);
  ~number() override = default;

  value_type eval() const;

 private:
  value_type value_;
};

class variable: public expression {
 public:
  variable(const std::string& str);
  variable(std::string&& str);
  ~variable() override = default;

  int eval() const;

 private:
  std::string name_;
};

class ctrl_statement: public statement {
 public:
  ~ctrl_statement() override = default;

  ctrl_statement(CtrlStatement type, expression* cond, statement_block* body);
 protected:
  CtrlStatement type_;
  expression* condition_;
  statement_block* body_;
};

class bin_operator: public expression {
 public:
  bin_operator(BinOp type, pointer_type left, pointer_type right);
  ~bin_operator() override = default;

  int eval() const override;
 private:
  BinOp type_;
  pointer_type left_, right_;
};

class un_operator: public expression {
 public:
  un_operator(UnOp type, pointer_type child);
  ~un_operator() override = default;

  int eval() const override;
 private:
  UnOp type_;
  pointer_type child_;
};


} // <--- namespace ast

} // <--- namespace frontend
