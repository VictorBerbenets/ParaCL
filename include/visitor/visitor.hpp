#pragma once

#include <memory>
#include <vector>

#include "values.hpp"

namespace paracl {

namespace ast {

class ArrayAccessAssignment;
class statement;
class root_statement_block;
class statement_block;
class logic_expression;
class calc_expression;
class assignment;
class definition;
class un_operator;
class number;
class variable;
class if_operator;
class while_operator;
class print_function;
class read_expression;
class ArrayAccess;
class PresetArray;
class UniformArray;
class ArrayHolder;

} // namespace ast

class ValueWrapper;

class VisitorBase {
public:
  using ResultTy = ValueWrapper &;

  VisitorBase() = default;
  VisitorBase(const VisitorBase &) = delete;
  VisitorBase &operator=(const VisitorBase &) = delete;
  VisitorBase(VisitorBase &&) = default;
  VisitorBase &operator=(VisitorBase &&) = default;
  virtual ~VisitorBase() {};

  virtual ResultTy visit(ast::root_statement_block *stm) = 0;
  virtual ResultTy visit(ast::statement_block *StmBlock) = 0;
  virtual ResultTy visit(ast::calc_expression *CalcExpr) = 0;
  virtual ResultTy visit(ast::logic_expression *LogExpr) = 0;
  virtual ResultTy visit(ast::un_operator *UnOper) = 0;
  virtual ResultTy visit(ast::number *Num) = 0;
  virtual ResultTy visit(ast::variable *Var) = 0;
  virtual ResultTy visit(ast::assignment *Assign) = 0;
  virtual ResultTy visit(ast::if_operator *If) = 0;
  virtual ResultTy visit(ast::while_operator *While) = 0;
  virtual ResultTy visit(ast::read_expression *ReadExpr) = 0;
  virtual ResultTy visit(ast::print_function *Print) = 0;
  virtual ResultTy visit(ast::ArrayHolder *InitListArr) = 0;
  virtual ResultTy visit(ast::PresetArray *InitListArr) = 0;
  virtual ResultTy visit(ast::ArrayAccess *ArrAccess) = 0;
  virtual ResultTy visit(ast::UniformArray *Arr) = 0;
  virtual ResultTy visit(ast::ArrayAccessAssignment *Arr) = 0;

protected:
  virtual ResultTy acceptASTNode(ast::statement *Stm) = 0;

  template <DerivedFromValueWrapper ResType, typename... ArgsTy>
  ResType &createWrapperRef(ArgsTy &&...Args) {
    WrapperStorage.emplace_back(
        std::make_unique<ResType>(std::forward<ArgsTy>(Args)...));
    return *static_cast<ResType *>(WrapperStorage.back().get());
  }

private:
  std::vector<std::unique_ptr<ValueWrapper>> WrapperStorage;
};

} // namespace paracl
