#include "expression.hpp"

namespace paracl {
namespace ast {

number::number(value_type num, yy::location loc)
    : expression{loc}, value_{num} {}

const number::value_type &number::get_value() const noexcept { return value_; }

void number::accept(VisitorBase *Vis) { Vis->visit(this); }

void number::accept(CodeGenVisitor *CodeGenVis) { CodeGenVis->visit(this); }

variable::variable(statement_block *curr_block, SymbNameType &&var_name,
                   yy::location l)
    : expression{curr_block, l}, name_{std::forward<SymbNameType>(var_name)} {}

void variable::accept(VisitorBase *Vis) { Vis->visit(this); }

void variable::accept(CodeGenVisitor *CodeGenVis) { CodeGenVis->visit(this); }

SymTabKey variable::entityKey() { return {name_, scope()}; }

const SymbNameType &variable::name() const noexcept { return name_; }

un_operator::un_operator(UnOp type, pointer_type arg, yy::location loc)
    : expression{loc}, type_{type}, arg_{arg} {}

void un_operator::accept(VisitorBase *Vis) { Vis->visit(this); }

void un_operator::accept(CodeGenVisitor *CodeGenVis) {
  CodeGenVis->visit(this);
}

expression *un_operator::arg() noexcept { return arg_; }
UnOp un_operator::type() const noexcept { return type_; }

void calc_expression::accept(VisitorBase *Vis) { Vis->visit(this); }

void calc_expression::accept(CodeGenVisitor *CodeGenVis) {
  CodeGenVis->visit(this);
}

void logic_expression::accept(VisitorBase *Vis) { Vis->visit(this); }

void logic_expression::accept(CodeGenVisitor *CodeGenVis) {
  CodeGenVis->visit(this);
}

assignment::assignment(statement_block *curr_block, variable *LValue,
                       expression *expr, PCLType::TypeID ID, yy::location loc)
    : expression{curr_block, loc}, LValue(LValue), Identifier{expr}, ID(ID) {}

void assignment::accept(VisitorBase *VisitorBase) { VisitorBase->visit(this); }

void assignment::accept(CodeGenVisitor *CodeGenVis) { CodeGenVis->visit(this); }

variable *assignment::getLValue() noexcept { return LValue; }
expression *assignment::getIdentExp() noexcept { return Identifier; }

SymbNameType assignment::name() const { return LValue->name(); }

SymTabKey assignment::entityKey() {
  assert(LValue);
  return LValue->entityKey();
}

PCLType::TypeID assignment::getID() const noexcept { return ID; }

read_expression::read_expression(yy::location loc) : expression{loc} {}

void read_expression::accept(VisitorBase *VisitorBase) {
  VisitorBase->visit(this);
}

void read_expression::accept(CodeGenVisitor *CodeGenVis) {
  CodeGenVis->visit(this);
}

} // namespace ast
} // namespace paracl
