#include "expression.hpp"

namespace paracl {
namespace ast {

using ResultValue = expression::ResultValue;

number::number(value_type num, yy::location loc)
    : expression{loc}, value_{num} {}

const number::value_type &number::get_value() const noexcept { return value_; }

ResultValue number::accept(VisitorBasePtr Vis) { return Vis->visit(this); }

variable::variable(statement_block *curr_block, SymbNameType &&var_name,
                   yy::location l)
    : expression{curr_block, l}, name_{std::forward<SymbNameType>(var_name)} {}

ResultValue variable::accept(VisitorBasePtr Vis) { return Vis->visit(this); }

SymTabKey variable::entityKey() { return {name_, scope()}; }

llvm::StringRef variable::name() const noexcept { return name_; }

un_operator::un_operator(UnOp type, pointer_type arg, yy::location loc)
    : expression{loc}, type_{type}, arg_{arg} {}

ResultValue un_operator::accept(VisitorBasePtr Vis) { return Vis->visit(this); }

expression *un_operator::arg() noexcept { return arg_; }
UnOp un_operator::type() const noexcept { return type_; }

ResultValue calc_expression::accept(VisitorBasePtr Vis) {
  return Vis->visit(this);
}

ResultValue logic_expression::accept(VisitorBasePtr Vis) {
  return Vis->visit(this);
}

assignment::assignment(statement_block *curr_block, variable *LValue,
                       expression *expr, yy::location loc)
    : expression{curr_block, loc}, LValue(LValue), Identifier{expr} {}

ResultValue assignment::accept(VisitorBasePtr VisitorBase) {
  return VisitorBase->visit(this);
}

variable *assignment::getLValue() noexcept { return LValue; }
expression *assignment::getIdentExp() noexcept { return Identifier; }

llvm::StringRef assignment::name() const { return LValue->name(); }

SymTabKey assignment::entityKey() {
  assert(LValue);
  return LValue->entityKey();
}

read_expression::read_expression(yy::location loc) : expression{loc} {}

ResultValue read_expression::accept(VisitorBasePtr VisitorBase) {
  return VisitorBase->visit(this);
}

} // namespace ast
} // namespace paracl
