#include <iostream>
#include <stdexcept>

#include "ast_includes.hpp"
#include "identifiers.hpp"
#include "interpreter.hpp"

namespace paracl {

void interpreter::visit(ast::root_statement_block *StmBlock) {
  for (auto &&statement : *StmBlock) {
    statement->accept(this);
  }
}

void interpreter::visit(ast::statement_block *StmBlock) {
  for (auto &&statement : *StmBlock) {
    statement->accept(this);
  }
}

void interpreter::visit(ast::calc_expression *CalcExpr) {
  CalcExpr->left()->accept(this);
  auto lhs = get_value();
  CalcExpr->right()->accept(this);
  auto rhs = get_value();

  switch (CalcExpr->type()) {
  case ast::CalcOp::ADD:
    set_value(lhs + rhs);
    break;
  case ast::CalcOp::SUB:
    set_value(lhs - rhs);
    break;
  case ast::CalcOp::MUL:
    set_value(lhs * rhs);
    break;
  case ast::CalcOp::PERCENT:
    set_value(lhs % rhs);
    break;
  case ast::CalcOp::DIV:
    if (auto check = rhs; check) {
      set_value(lhs / check);
    } else {
      throw std::runtime_error{"trying to divide by 0"};
    }
    break;
  default:
    throw std::logic_error{"unrecognized logic type"};
  }
}

void interpreter::visit(ast::un_operator *UnOp) {
  UnOp->arg()->accept(this);

  switch (UnOp->type()) {
  case ast::UnOp::PLUS:
    set_value(+get_value());
    break;
  case ast::UnOp::MINUS:
    set_value(-get_value());
    break;
  case ast::UnOp::NEGATE:
    set_value(!get_value());
    break;
  default:
    throw std::logic_error{"unrecognized logic type"};
  }
}

void interpreter::visit(ast::logic_expression *LogExpr) {
  LogExpr->left()->accept(this);
  auto lhs = get_value();
  LogExpr->right()->accept(this);
  auto rhs = get_value();

  switch (LogExpr->type()) {
  case ast::LogicOp::LESS:
    set_value(lhs < rhs);
    break;
  case ast::LogicOp::LESS_EQ:
    set_value(lhs <= rhs);
    break;
  case ast::LogicOp::LOGIC_AND:
    set_value(lhs && rhs);
    break;
  case ast::LogicOp::LOGIC_OR:
    set_value(lhs || rhs);
    break;
  case ast::LogicOp::GREATER:
    set_value(lhs > rhs);
    break;
  case ast::LogicOp::GREATER_EQ:
    set_value(lhs >= rhs);
    break;
  case ast::LogicOp::EQ:
    set_value(lhs == rhs);
    break;
  case ast::LogicOp::NEQ:
    set_value(lhs != rhs);
    break;
  default:
    throw std::logic_error{"unrecognized logic type"};
  }
}

void interpreter::visit(ast::number *Num) { set_value(Num->get_value()); }

void interpreter::visit(ast::variable *Var) {
  auto *curr_scope = Var->scope();
  auto *DeclScope = SymTbl.getDeclScopeFor(SymbNameType(Var->name()), Var->scope());
  auto *Ty = SymTbl.getTypeFor(SymbNameType(Var->name()), Var->scope());
  assert(DeclScope);
  assert(Ty);
  auto *Value = ValManager.getValueFor({SymbNameType(Var->name()), DeclScope});
  if (Ty->getTypeID() == PCLType::TypeID::Int32) {
    auto Val = static_cast<IntegerVal*>(Value)->getValue();
#if 0
    std::cout << "SETTED VALUE = " << Val << std::endl;
#endif
    set_value(Val);
  }
#if 0
  set_value();
  set_value(right_scope->value(Var->name()));
#endif
}

void interpreter::visit(ast::if_operator *If) {
  If->condition()->accept(this);

  if (get_value()) {
    If->body()->accept(this);
  } else if (If->else_block()) {
    If->else_block()->accept(this);
  }
}

void interpreter::visit(ast::while_operator *While) {
  While->condition()->accept(this);

  while (get_value()) {
    While->body()->accept(this);
    While->condition()->accept(this);
  }
}

void interpreter::visit(ast::read_expression * /* unused */) {
  int tmp{0};
  input_stream_ >> tmp;
  set_value(tmp);
}

void interpreter::visit(ast::print_function *Print) {
  Print->get()->accept(this);
  output_stream_ << get_value() << std::endl;
}

void interpreter::visit(ast::assignment *Assign) {
  //  set_value(evaluate(Assign->ident_exp());
  Assign->ident_exp()->accept(this);
  ValManager.createValueFor<IntegerVal>({SymbNameType(Assign->name()), Assign->scope()}, get_value());
}

} // namespace paracl
