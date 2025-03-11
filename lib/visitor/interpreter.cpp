#include <llvm/Support/FormatVariadic.h>

#include <iostream>
#include <sstream>

#include "ast_includes.hpp"
#include "identifiers.hpp"
#include "interpreter.hpp"

namespace paracl {

InterpreterBase::ValueTypePtr
InterpreterBase::performLogicalOperation(ast::LogicOp Op, IntegerVal *Lhs,
                                         IntegerVal *Rhs, IntegerTy *Type) {
  assert(Lhs);
  assert(Rhs);
  assert(Type);
  switch (Op) {
  case ast::LogicOp::LESS:
    return ValManager.createValue<IntegerVal>(*Lhs < *Rhs, Type);
  case ast::LogicOp::LESS_EQ:
    return ValManager.createValue<IntegerVal>(*Lhs <= *Rhs, Type);
    break;
  case ast::LogicOp::AND:
    return ValManager.createValue<IntegerVal>(*Lhs && *Rhs, Type);
    break;
  case ast::LogicOp::OR:
    return ValManager.createValue<IntegerVal>(*Lhs || *Rhs, Type);
    break;
  case ast::LogicOp::GREATER:
    return ValManager.createValue<IntegerVal>(*Lhs > *Rhs, Type);
    break;
  case ast::LogicOp::GREATER_EQ:
    return ValManager.createValue<IntegerVal>(*Lhs >= *Rhs, Type);
    break;
  case ast::LogicOp::EQ:
    return ValManager.createValue<IntegerVal>(*Lhs == *Rhs, Type);
    break;
  case ast::LogicOp::NEQ:
    return ValManager.createValue<IntegerVal>(*Lhs != *Rhs, Type);
    break;
  default:
    llvm_unreachable("Unsupported logic operator");
  }
}

InterpreterBase::ValueTypePtr
InterpreterBase::performUnaryOperation(ast::UnOp Op, IntegerVal *Value,
                                       IntegerTy *Type) {
  assert(Value);
  assert(Type);
  switch (Op) {
  case ast::UnOp::PLUS:
    return Value;
    break;
  case ast::UnOp::MINUS:
    return ValManager.createValue<IntegerVal>(-*Value, Type);
    break;
  case ast::UnOp::NEGATE:
    return ValManager.createValue<IntegerVal>(!*Value, Type);
    break;
  default:
    llvm_unreachable("Unsupported unary operator");
  }
}

InterpreterBase::ValueTypePtr
InterpreterBase::performArithmeticOperation(ast::CalcOp Op, IntegerVal *Lhs,
                                            IntegerVal *Rhs, IntegerTy *Type,
                                            yy::location Loc) {
  assert(Lhs);
  assert(Rhs);
  assert(Type);
  switch (Op) {
  case ast::CalcOp::ADD:
    return ValManager.createValue<IntegerVal>(*Lhs + *Rhs, Type);
    break;
  case ast::CalcOp::SUB:
    return ValManager.createValue<IntegerVal>(*Lhs - *Rhs, Type);
    break;
  case ast::CalcOp::MUL:
    return ValManager.createValue<IntegerVal>(*Lhs * *Rhs, Type);
    break;
  case ast::CalcOp::PERCENT:
    return ValManager.createValue<IntegerVal>(*Lhs % *Rhs, Type);
    break;
  case ast::CalcOp::DIV:
    if (int check = *Rhs; check) {
      return ValManager.createValue<IntegerVal>(*Lhs / check, Type);
    } else {
      std::stringstream Str;
      Str << Loc;
      throw std::runtime_error{
          llvm::formatv("{0}, trying to divide by 0", Str.str())};
    }
    break;
  default:
    llvm_unreachable("Unsupported calculation operator");
  }
}

void Interpreter::visit(ast::statement_block *StmBlock) {
  for (auto &&statement : *StmBlock) {
    statement->accept(this);
  }
  freeResources(StmBlock);
}

void Interpreter::visit(ast::calc_expression *CalcExp) {
  auto *Lhs = getValueAfterAccept<IntegerVal>(CalcExp->left());
  auto *Rhs = getValueAfterAccept<IntegerVal>(CalcExp->right());
  auto *Type = Lhs->getType();
  assert(Type->isInt32Ty());

  setValue(performArithmeticOperation(CalcExp->type(), Lhs, Rhs, Type,
                                      CalcExp->location()));
}

void Interpreter::visit(ast::un_operator *UnOp) {
  auto *Value = getValueAfterAccept<IntegerVal>(UnOp->arg());
  auto *Type = Value->getType();
  assert(Type->isInt32Ty());

  setValue(performUnaryOperation(UnOp->type(), Value, Type));
}

void Interpreter::visit(ast::logic_expression *LogExp) {
  auto *Lhs = getValueAfterAccept<IntegerVal>(LogExp->left());
  auto *Type = Lhs->getType();
  assert(Type->isInt32Ty());
  assert(Lhs);
  if (LogExp->type() == ast::LogicOp::AND && !*Lhs) {
    setValue(ValManager.createValue<IntegerVal>(0, Type));
  } else if (LogExp->type() == ast::LogicOp::OR && *Lhs) {
    setValue(ValManager.createValue<IntegerVal>(1, Type));
  } else {
    auto *Rhs = getValueAfterAccept<IntegerVal>(LogExp->right());
    setValue(performLogicalOperation(LogExp->type(), Lhs, Rhs, Type));
  }
}

void Interpreter::visit(ast::number *Num) {
  setValue(ValManager.createValue<IntegerVal>(
      Num->get_value(), SymTbl.createType(TypeID::Int32)));
}

void Interpreter::visit(ast::variable *Var) {
  auto EntityKey = Var->entityKey();
  [[maybe_unused]] auto *Ty = SymTbl.getTypeFor(EntityKey);
  assert(Ty);
  auto *Value = ValManager.getValueFor(SymTbl.getDeclKeyFor(EntityKey));
  setValue(Value);
}

void Interpreter::visit(ast::if_operator *If) {
  auto *CondVal = getValueAfterAccept<IntegerVal>(If->condition());
  if (CondVal->getValue()) {
    If->body()->accept(this);
  } else if (If->else_block()) {
    If->else_block()->accept(this);
  }
}

void Interpreter::visit(ast::while_operator *While) {
  auto *CondVal = getValueAfterAccept<IntegerVal>(While->condition());
  assert(CondVal);
  while (CondVal->getValue()) {
    While->body()->accept(this);
    CondVal = getValueAfterAccept<IntegerVal>(While->condition());
  }
}

void Interpreter::visit(ast::read_expression * /* unused */) {
  int Tmp = 0;
  input_stream_ >> Tmp;
  setValue(ValManager.createValue<IntegerVal>(
      Tmp, SymTbl.createType(TypeID::Int32)));
}

void Interpreter::visit(ast::print_function *Print) {
  auto *Val = getValueAfterAccept(Print->get());
  output_stream_ << static_cast<IntegerVal *>(Val)->getValue() << std::endl;
}

void Interpreter::visit(ast::assignment *Assign) {
  auto *IdentExp = getValueAfterAccept(Assign->getIdentExp());
  assert(IdentExp);
  auto *IdentType = IdentExp->getType();
  auto EntityKey = Assign->entityKey();
  if (!SymTbl.isDefined(EntityKey)) {
    [[maybe_unused]] auto IsDefined =
        SymTbl.tryDefine(EntityKey, IdentExp->getType());
    assert(IsDefined);
    auto DeclKey = SymTbl.getDeclKeyFor(EntityKey);
    if (IdentType->isArrayTy()) {
      // Since the array is assigned only during its creation, no copies are
      // made; instead, the created array is immediately bound to the name
      ValManager.linkValueWithName(DeclKey, IdentExp);
    } else if (IdentType->isInt32Ty()) {
      ValManager.createValueFor<IntegerVal>(
          DeclKey, static_cast<IntegerVal *>(IdentExp)->getValue(),
          SymTbl.createType(TypeID::Int32));
    }
  } else {
    auto *Value =
        ValManager.getValueFor<IntegerVal>(SymTbl.getDeclKeyFor(EntityKey));
    assert(Value);
    Value->setValue(static_cast<IntegerVal *>(IdentExp));
  }
}
  
void Interpreter::visit(ast::ArrayStore *ArrStore) {
  auto *Arr = ArrStore->get();
  assert(Arr);
  Arr->accept(this);
}

void Interpreter::visit(ast::PresetArray *PresetArr) {
  llvm::SmallVector<PCLValue *> PresetValues;
  PresetValues.reserve(PresetArr->size());
  for (auto *CurrExp : *PresetArr) {
    PresetValues.push_back(getValueAfterAccept(CurrExp));
  }

  auto *ArrVal = ValManager.createValue<PresetArrayVal>(
      PresetValues.begin(), PresetValues.end(),
      static_cast<ArrayTy *>(SymTbl.createType(TypeID::PresetArray)));
  addResourceForFree(ArrVal, PresetArr->scope());
  setValue(ArrVal);
}

void Interpreter::visit(ast::ArrayAccess *ArrAccess) {
  auto AccessSize = ArrAccess->getSize();
  auto *CurrArr = ValManager.getValueFor<ArrayBase>(
      SymTbl.getDeclKeyFor(ArrAccess->entityKey()));
  assert(CurrArr);
  int CurrID = 0;
  for (unsigned ArrID = 1; auto RankID : *ArrAccess) {
    auto *RankVal = getValueAfterAccept<IntegerVal>(RankID);
    assert(RankVal);
    CurrID = RankVal->getValue();
    if (ArrID != AccessSize)
      CurrArr = static_cast<ArrayBase *>((*CurrArr)[CurrID]);
    ArrID++;
  }
  assert(CurrArr);
  setValue((*CurrArr)[CurrID]);
}

void Interpreter::visit(ast::UniformArray *UnifArr) {
  auto *InitExpr = getValueAfterAccept(UnifArr->getInitExpr());
  auto *Size = getValueAfterAccept<IntegerVal>(UnifArr->getSize());

  assert(InitExpr && Size);
  auto *ArrVal = ValManager.createValue<UniformArrayVal>(
      InitExpr, *Size,
      static_cast<ArrayTy *>(SymTbl.createType(TypeID::UniformArray)));
  addResourceForFree(ArrVal, UnifArr->scope());
  setValue(ArrVal);
}

void Interpreter::visit(ast::ArrayAccessAssignment *Arr) {
  auto *LValue = getValueAfterAccept<IntegerVal>(Arr->getArrayAccess());
  auto *IdentExp = getValueAfterAccept<IntegerVal>(Arr->getIdentExp());

  assert(LValue && IdentExp);
  LValue->setValue(IdentExp);
}

void Interpreter::freeResources(ast::statement_block *StmBlock) {
  assert(StmBlock);
  if (ResourceHandleMap.contains(StmBlock))
    llvm::for_each(ResourceHandleMap[StmBlock], [](auto *Val) {
      assert(Val);
      auto *Type = Val->getType();
      assert(Type);
      if (Type->isArrayTy()) {
        static_cast<ArrayBase *>(Val)->destroy();
      }
    });
}

void Interpreter::addResourceForFree(PCLValue *ValToFree,
                                     ast::statement_block *ScopeToFree) {
  assert(ScopeToFree);
  ResourceHandleMap[ScopeToFree].push_back(ValToFree);
}

} // namespace paracl
