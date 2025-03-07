#pragma once

#include "identifiers.hpp"
#include "location.hh"
#include "statement.hpp"
#include "visitor.hpp"

namespace paracl {

namespace ast {

class expression : public statement {
protected:
  using statement::statement;

  expression() = default;

  using pointer_type = expression *;
};

class number : public expression {
  using value_type = int;

public:
  number(value_type num, yy::location loc);

  const value_type &get_value() const noexcept;

  void accept(base_visitor *b_visitor) override;

  void accept(CodeGenVisitor *CodeGenVis) override;

private:
  value_type value_;
};

class variable : public expression {
public:
  variable(statement_block *curr_block, const std::string &var_name,
           yy::location l);

  variable(statement_block *curr_block, std::string &&var_name, yy::location l);

  void declare();

  void accept(base_visitor *b_visitor) override;

  void accept(CodeGenVisitor *CodeGenVis) override;

  const std::string &name() const noexcept;

private:
  std::string name_;
};

class un_operator : public expression {
public:
  un_operator(UnOp type, pointer_type arg, yy::location loc);

  void accept(base_visitor *b_visitor) override;

  void accept(CodeGenVisitor *CodeGenVis) override;

  expression *arg() noexcept;
  UnOp type() const noexcept;

private:
  UnOp type_;
  pointer_type arg_;
};

template <typename BinType> class bin_operator : public expression {
public:
  bin_operator(BinType type, pointer_type left, pointer_type right,
               yy::location loc)
      : expression{loc}, type_{type}, left_{left}, right_{right} {}

  pointer_type left() noexcept { return left_; }
  pointer_type right() noexcept { return right_; }
  BinType type() const noexcept { return type_; }

protected:
  BinType type_;
  pointer_type left_, right_;
};

class calc_expression : public bin_operator<CalcOp> {
public:
  using bin_operator::bin_operator;

  void accept(base_visitor *b_visitor) override;
  void accept(CodeGenVisitor *CodeGenVis) override;
};

class logic_expression : public bin_operator<LogicOp> {
public:
  using bin_operator::bin_operator;

  void accept(base_visitor *b_visitor) override;
  void accept(CodeGenVisitor *CodeGenVis) override;
};

class assignment : public expression {
public:
  assignment(statement_block *curr_block, const std::string &name,
             expression *expr, yy::location loc);

  assignment(statement_block *curr_block, std::string &&name, expression *expr,
             yy::location loc);

  void accept(base_visitor *base_visitor) override;
  void accept(CodeGenVisitor *CodeGenVis) override;

  expression *ident_exp() noexcept;

  void redefine(int value);

  const std::string &name() const noexcept;

private:
  std::string name_;
  expression *identifier_;
};

class read_expression : public expression {
  using value_type = int;

public:
  read_expression(yy::location loc);

  void accept(base_visitor *base_visitor) override;
  void accept(CodeGenVisitor *CodeGenVis) override;
};

} // namespace ast

} // namespace paracl
