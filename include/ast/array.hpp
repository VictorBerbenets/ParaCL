#pragma once

#include "expression.hpp"

namespace paracl {
namespace ast {

class PresetArray : public expression {
public:
  using expression::expression;
  template <typename Iter>
  PresetArray(statement_block *StmBlock, Iter Begin, Iter End,
                yy::location Loc)
      : expression(StmBlock, Loc), Elements(Begin, End) {}

  void accept(base_visitor *b_visitor) override { b_visitor->visit(this); }
  void accept(CodeGenVisitor *CodeGenVis) override { CodeGenVis->visit(this); }

  auto begin() { return Elements.begin(); }
  auto end() { return Elements.end(); }
  auto begin() const { return Elements.begin(); }
  auto end() const { return Elements.end(); }

  unsigned size() const { return Elements.size(); }

private:
  llvm::SmallVector<expression *> Elements;
};

class UniformArray : public expression {
public:
  using expression::expression;

  UniformArray(statement_block *StmBlock, expression *InitExpr, expression *Size,
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
  ArrayAccess(statement_block *StmBlock, SymbNameType &&Name, Iter Begin,
              Iter End, yy::location loc)
      : variable(StmBlock, std::forward<SymbNameType>(Name), loc),
        RanksId(Begin, End) {}

  void accept(base_visitor *b_visitor) override { b_visitor->visit(this); }
  void accept(CodeGenVisitor *CodeGenVis) override { CodeGenVis->visit(this); }

  void setIdentExp(expression *Ident) { IdentExp = Ident; }

  unsigned getSize() const noexcept { return RanksId.size(); }
  expression *getIdentExp() const noexcept { return IdentExp; }

  auto begin() { return RanksId.begin(); }
  auto end() { return RanksId.end(); }
  auto begin() const { return RanksId.begin(); }
  auto end() const { return RanksId.end(); }

private:
  llvm::SmallVector<expression *> RanksId;
  expression *IdentExp;
};

class ArrayAccessAssignment : public expression {
public:
  ArrayAccessAssignment(statement_block *StmBlock, ArrayAccess *Access,
                        expression *Ident, yy::location Loc)
      : expression(StmBlock, Loc), ArrAccess(Access), Identifier(Ident) {}

  void accept(base_visitor *b_visitor) override { b_visitor->visit(this); }
  void accept(CodeGenVisitor *CodeGenVis) override { CodeGenVis->visit(this); }

  ArrayAccess *getArrayAccess() noexcept { return ArrAccess; }
  expression *getIdentExp() noexcept { return Identifier; }

private:
  ArrayAccess *ArrAccess;
  expression *Identifier;
};

} // namespace ast
} // namespace paracl
