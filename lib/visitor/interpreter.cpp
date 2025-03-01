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
  auto *LhsType = get_value()->getType();
  assert(LhsType->isInt32Ty());
  auto *Lhs = static_cast<IntegerVal *>(get_value());

  CalcExpr->right()->accept(this);
  auto *RhsType = get_value()->getType();
  assert(RhsType->isInt32Ty());
  auto *Rhs = static_cast<IntegerVal *>(get_value());
  switch (CalcExpr->type()) {
  case ast::CalcOp::ADD:
    set_value(ValManager.createValue<IntegerVal>(*Lhs + *Rhs, LhsType));
    break;
  case ast::CalcOp::SUB:
    set_value(ValManager.createValue<IntegerVal>(*Lhs - *Rhs, LhsType));
    break;
  case ast::CalcOp::MUL:
    set_value(ValManager.createValue<IntegerVal>(*Lhs * *Rhs, LhsType));
    break;
  case ast::CalcOp::PERCENT:
    set_value(ValManager.createValue<IntegerVal>(*Lhs % *Rhs, LhsType));
    break;
  case ast::CalcOp::DIV:
    if (int check = *Rhs; check) {
      set_value(
          ValManager.createValue<IntegerVal>(*Lhs / check, LhsType));
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

  auto *Type = get_value()->getType();
  assert(Type->isInt32Ty());
  auto *Value = static_cast<IntegerVal *>(get_value());
  switch (UnOp->type()) {
  case ast::UnOp::PLUS:
    set_value(Value);
    break;
  case ast::UnOp::MINUS:
    set_value(ValManager.createValue<IntegerVal>(-*Value, Type));
    break;
  case ast::UnOp::NEGATE:
    set_value(ValManager.createValue<IntegerVal>(!*Value, Type));
    break;
  default:
    throw std::logic_error{"unrecognized logic type"};
  }
}

void interpreter::visit(ast::logic_expression *LogExpr) {
  LogExpr->left()->accept(this);
  auto *LhsType = get_value()->getType();
  assert(LhsType->isInt32Ty());
  auto *Lhs = static_cast<IntegerVal *>(get_value());

  LogExpr->right()->accept(this);
  auto *RhsType = get_value()->getType();
  assert(RhsType->isInt32Ty());
  auto *Rhs = static_cast<IntegerVal *>(get_value());

  switch (LogExpr->type()) {
  case ast::LogicOp::LESS:
    set_value(ValManager.createValue<IntegerVal>(*Lhs < *Rhs, LhsType));
    break;
  case ast::LogicOp::LESS_EQ:
    set_value(
        ValManager.createValue<IntegerVal>(*Lhs <= *Rhs, LhsType));
    break;
  case ast::LogicOp::LOGIC_AND:
    set_value(
        ValManager.createValue<IntegerVal>(*Lhs && *Rhs, LhsType));
    break;
  case ast::LogicOp::LOGIC_OR:
    set_value(
        ValManager.createValue<IntegerVal>(*Lhs || *Rhs, LhsType));
    break;
  case ast::LogicOp::GREATER:
    set_value(ValManager.createValue<IntegerVal>(*Lhs > *Rhs, LhsType));
    break;
  case ast::LogicOp::GREATER_EQ:
    set_value(
        ValManager.createValue<IntegerVal>(*Lhs >= *Rhs, LhsType));
    break;
  case ast::LogicOp::EQ:
    set_value(
        ValManager.createValue<IntegerVal>(*Lhs == *Rhs, LhsType));
    break;
  case ast::LogicOp::NEQ:
    set_value(
        ValManager.createValue<IntegerVal>(*Lhs != *Rhs, LhsType));
    break;
  default:
    throw std::logic_error{"unrecognized logic type"};
  }
}

void interpreter::visit(ast::number *Num) {
  set_value(ValManager.createValue<IntegerVal>(
      Num->get_value(), SymTbl.getType(TypeID::Int32)));
}

void interpreter::visit(ast::variable *Var) {
  auto *DeclScope =
      SymTbl.getDeclScopeFor(SymbNameType(Var->name()), Var->scope());
  auto *Ty = SymTbl.getTypeFor(SymbNameType(Var->name()), Var->scope());
  assert(DeclScope && Ty);
#if 0
  std::cout << "VAR NAME = " << Var->name() << "; DeclScope = " << DeclScope << '\n';
#endif
  auto *Value = ValManager.getValueFor({SymbNameType(Var->name()), DeclScope});
#if 0
  if (Ty->isInt32Ty()) {
  
  }
#endif
  set_value(static_cast<IntegerVal *>(Value));
}

void interpreter::visit(ast::if_operator *If) {
  If->condition()->accept(this);

#if 0
  auto *Val = get_value();
  assert(Val);
  assert(Val->getType()->isInt32Ty());
  auto *IntVal = static_cast<IntegerVal*>(Val);
#endif
  if (static_cast<IntegerVal *>(get_value())->getValue()) {
    If->body()->accept(this);
  } else if (If->else_block()) {
    If->else_block()->accept(this);
  }
}

void interpreter::visit(ast::while_operator *While) {
  While->condition()->accept(this);
  assert(get_value());
  while (static_cast<IntegerVal *>(get_value())->getValue()) {
    While->body()->accept(this);
    While->condition()->accept(this);
  }
}

void interpreter::visit(ast::read_expression * /* unused */) {
  int tmp{0};
  input_stream_ >> tmp;
  set_value(ValManager.createValue<IntegerVal>(
      tmp, SymTbl.getType(TypeID::Int32)));
}

void interpreter::visit(ast::print_function *Print) {
  Print->get()->accept(this);
  auto *Val = get_value();
  assert(Val);
  output_stream_ << static_cast<IntegerVal *>(Val)->getValue() << std::endl;
}

void interpreter::visit(ast::assignment *Assign) {
  Assign->getIdentExp()->accept(this);
  auto *IdentExp = get_value();
  SymbNameType Name(Assign->getLValue()->name()); 
  auto *DeclScope =
      SymTbl.getDeclScopeFor(Name, Assign->scope());
  ValManager.linkValueWithName({Name, DeclScope}, IdentExp);
}

void interpreter::visit(ast::InitListArray *InitListArr) {}

void interpreter::visit(ast::ArrayAccess *ArrAccess) {
  SymbNameType Name(ArrAccess->name());
  auto *DeclScope = SymTbl.getDeclScopeFor(Name,
                                           ArrAccess->scope());
  auto *ArrTy =
      SymTbl.getTypeFor(Name, ArrAccess->scope());

  auto AccessSize = ArrAccess->getSize();
  llvm::SmallVector<unsigned> Ids;
  Ids.reserve(AccessSize);
  auto *CurrArr = static_cast<ArrayVal*>(ValManager.getValueFor({Name, DeclScope}));
  int CurrID = 0;
  for (unsigned ArrID = 1; auto RankId : *ArrAccess) {
    RankId->accept(this);
    assert(get_value()->getType()->isInt32Ty());
    CurrID = static_cast<IntegerVal*>(get_value())->getValue();
#if 0
    std::cout << "CurrID in CYCLE = " << CurrID << '\n';
#endif
    if (ArrID != AccessSize)
      CurrArr = static_cast<ArrayVal*>((*CurrArr)[CurrID]); 
    ArrID++;
  }
#if 0
  std::cout << "CurrID = " << CurrID << '\n';
  std::cout << "ARRAY ADDRESS FOR ACCESS = " << std::hex << CurrArr << std::dec << '\n';
#endif
  set_value((*CurrArr)[CurrID]);
}

void interpreter::visit(ast::UndefVar *UndVar) { set_value(ValManager.createValue<IntegerVal>(0, SymTbl.getType(TypeID::Int32))); }

void interpreter::visit(ast::Array *Arr) {
  Arr->getInitExpr()->accept(this);
  auto *InitExpr = get_value();
  Arr->getSize()->accept(this);
  auto *Size = get_value();
  assert(Size->getType()->isInt32Ty()); 

  set_value(ValManager.createValue<ArrayVal>(InitExpr, static_cast<IntegerVal*>(Size), SymTbl.getType(TypeID::Array)));
}
  
void interpreter::visit(ast::ArrayAccessAssignment *Arr) {
  Arr->getArrayAccess()->accept(this);
  auto *LValue = static_cast<IntegerVal*>(get_value());
  Arr->getIdentExp()->accept(this);
  auto *IdentExp = static_cast<IntegerVal*>(get_value());
  
  LValue->setValue(IdentExp);
}

} // namespace paracl
