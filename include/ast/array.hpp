#pragma once

#include "expression.hpp"
#include "types.hpp"

namespace paracl {
namespace ast {

class InitListArray : public expression {
public:
  using expression::expression;
  template <typename Iter>
  InitListArray(statement_block *StmBlock, Iter Begin, Iter End,
                yy::location Loc)
      : expression(StmBlock, Loc), Elements(Begin, End) {}

  void accept(base_visitor *b_visitor) override { b_visitor->visit(this); }
  void accept(CodeGenVisitor *CodeGenVis) override { CodeGenVis->visit(this); }

private:
  llvm::SmallVector<expression *> Elements;
};

class Array : public expression {
public:
  using expression::expression;

  Array(statement_block *StmBlock, expression *InitExpr, expression *Size,
        yy::location Loc)
      : expression(StmBlock, Loc), InitExpr(InitExpr), Size(Size) {}

  expression *getInitExpr() noexcept { return InitExpr; }
  expression *getSize() noexcept { return Size; }

  void accept(base_visitor *b_visitor) override { b_visitor->visit(this); }
  void accept(CodeGenVisitor *CodeGenVis) override { CodeGenVis->visit(this); }

private:
  expression *InitExpr;
  expression *Size;
};

class ArrayAccess : public variable {
public:
  template <typename Iter>
  ArrayAccess(statement_block *StmBlock, std::string Name, Iter Begin, Iter End,
              yy::location loc)
      : variable(StmBlock, Name, loc), RanksId(Begin, End) {}

  void accept(base_visitor *b_visitor) override { b_visitor->visit(this); }
  void accept(CodeGenVisitor *CodeGenVis) override { CodeGenVis->visit(this); }

  unsigned getSize() const noexcept { return RanksId.size(); }

  auto begin() { return RanksId.begin(); }
  auto end() { return RanksId.end(); }
  auto begin() const { return RanksId.begin(); }
  auto end() const { return RanksId.end(); }

private:
  llvm::SmallVector<expression *> RanksId;
};

class UndefVar : public expression {
public:
  UndefVar(statement_block *StmBlock, yy::location loc)
      : expression(StmBlock, loc) {}

  void accept(base_visitor *b_visitor) override { b_visitor->visit(this); }
  void accept(CodeGenVisitor *CodeGenVis) override { CodeGenVis->visit(this); }
};

} // namespace ast
} // namespace paracl
