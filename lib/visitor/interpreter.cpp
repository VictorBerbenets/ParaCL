#include <iostream>

#include "ast_includes.hpp"
#include "identifiers.hpp"
#include "interpreter.hpp"

namespace paracl {

void interpreter::visit(ast::statement_block *StmBlock) {
  for (auto &&statement : *StmBlock) {
    statement->accept(this);
  }
}

void interpreter::visit(ast::calc_expression *CalcExp) {
  auto *Lhs = getValueAfterAccept<IntegerVal>(CalcExp->left());
  auto *Rhs = getValueAfterAccept<IntegerVal>(CalcExp->right());
  auto *Type = Lhs->getType();
  assert(Type->isInt32Ty());

  set_value(performArithmeticOperation(CalcExp->type(), Lhs, Rhs, Type,
                                       CalcExp->location()));
}

void interpreter::visit(ast::un_operator *UnOp) {
  auto *Value = getValueAfterAccept<IntegerVal>(UnOp->arg());
  auto *Type = Value->getType();
  assert(Type->isInt32Ty());

  set_value(performUnaryOperation(UnOp->type(), Value, Type));
}

void interpreter::visit(ast::logic_expression *LogExp) {
  auto *Lhs = getValueAfterAccept<IntegerVal>(LogExp->left());
  auto *Rhs = getValueAfterAccept<IntegerVal>(LogExp->right());
  auto *Type = Lhs->getType();
  assert(Type->isInt32Ty());

  set_value(performLogicalOperation(LogExp->type(), Lhs, Rhs, Type));
}

void interpreter::visit(ast::number *Num) {
  set_value(ValManager.createValue<IntegerVal>(
      Num->get_value(), SymTbl.createType(TypeID::Int32)));
}

void interpreter::visit(ast::variable *Var) {
  auto EntityKey = Var->entityKey();
  [[maybe_unused]] auto *Ty = SymTbl.getTypeFor(EntityKey);
  assert(Ty);
  auto *Value = ValManager.getValueFor(SymTbl.getDeclKeyFor(EntityKey));
  set_value(Value);
}

void interpreter::visit(ast::if_operator *If) {
  auto *CondVal = getValueAfterAccept<IntegerVal>(If->condition());
  if (CondVal->getValue()) {
    If->body()->accept(this);
  } else if (If->else_block()) {
    If->else_block()->accept(this);
  }
}

void interpreter::visit(ast::while_operator *While) {
  auto *CondVal = getValueAfterAccept<IntegerVal>(While->condition());
  assert(CondVal);
  while (CondVal->getValue()) {
    While->body()->accept(this);
    CondVal = getValueAfterAccept<IntegerVal>(While->condition());
  }
}

void interpreter::visit(ast::read_expression * /* unused */) {
  int tmp{0};
  input_stream_ >> tmp;
  set_value(ValManager.createValue<IntegerVal>(
      tmp, SymTbl.createType(TypeID::Int32)));
}

void interpreter::visit(ast::print_function *Print) {
  auto *Val = getValueAfterAccept(Print->get());
  output_stream_ << static_cast<IntegerVal *>(Val)->getValue() << std::endl;
}

void interpreter::visit(ast::assignment *Assign) {
  auto *IdentExp = getValueAfterAccept(Assign->getIdentExp());
  auto EntityKey = Assign->entityKey();
  if (!SymTbl.isDefined(EntityKey)) {
    [[maybe_unused]] auto IsDefined =
        SymTbl.tryDefine(EntityKey, IdentExp->getType());
    assert(IsDefined);
  }
  ValManager.linkValueWithName(SymTbl.getDeclKeyFor(EntityKey), IdentExp);
}

void interpreter::visit(ast::PresetArray *InitListArr) {
  llvm::SmallVector<PCLValue *> PresetValues;
  PresetValues.reserve(InitListArr->size());
  for (auto *CurrExp : *InitListArr) {
    PresetValues.push_back(getValueAfterAccept(CurrExp));
  }

  set_value(ValManager.createValue<PresetArrayVal>(
      PresetValues.begin(), PresetValues.end(),
      static_cast<ArrayTy *>(SymTbl.createType(TypeID::PresetArray))));
}

void interpreter::visit(ast::ArrayAccess *ArrAccess) {
  auto AccessSize = ArrAccess->getSize();
  auto *CurrArr = ValManager.getValueFor<ArrayBase>(
      SymTbl.getDeclKeyFor(ArrAccess->entityKey()));
  int CurrID = 0;
  for (unsigned ArrID = 1; auto RankID : *ArrAccess) {
    auto *RankVal = getValueAfterAccept<IntegerVal>(RankID);
    assert(RankVal);
    CurrID = RankVal->getValue();
    if (ArrID != AccessSize)
      CurrArr = static_cast<ArrayBase *>((*CurrArr)[CurrID]);
    ArrID++;
  }
  set_value((*CurrArr)[CurrID]);
}

void interpreter::visit(ast::UniformArray *Arr) {
  auto *InitExpr = getValueAfterAccept(Arr->getInitExpr());
  auto *Size = getValueAfterAccept<IntegerVal>(Arr->getSize());

  assert(InitExpr && Size);
  set_value(ValManager.createValue<UniformArrayVal>(
      InitExpr, *Size,
      static_cast<ArrayTy *>(SymTbl.createType(TypeID::UniformArray))));
}

void interpreter::visit(ast::ArrayAccessAssignment *Arr) {
  auto *LValue = getValueAfterAccept<IntegerVal>(Arr->getArrayAccess());
  auto *IdentExp = getValueAfterAccept<IntegerVal>(Arr->getIdentExp());

  assert(LValue && IdentExp);
  LValue->setValue(IdentExp);
}

} // namespace paracl
