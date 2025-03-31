#include <llvm/Support/FormatVariadic.h>

#include <iostream>
#include <sstream>

#include "ast_includes.hpp"
#include "identifiers.hpp"
#include "interpreter.hpp"
#include "utils.hpp"

namespace paracl {

using ResultTy = InterpreterBase::ResultTy;

ResultTy InterpreterBase::performLogicalOperation(ast::LogicOp Op,
                                                  IntegerVal *Lhs,
                                                  IntegerVal *Rhs,
                                                  IntegerTy *Type) {
  assert(Lhs);
  assert(Rhs);
  assert(Type);
  switch (Op) {
  case ast::LogicOp::LESS:
    return createWrapperRef(
        ValManager.createValue<IntegerVal>(*Lhs < *Rhs, Type));
  case ast::LogicOp::LESS_EQ:
    return createWrapperRef(
        ValManager.createValue<IntegerVal>(*Lhs <= *Rhs, Type));
    break;
  case ast::LogicOp::AND:
    return createWrapperRef(
        ValManager.createValue<IntegerVal>(*Lhs && *Rhs, Type));
    break;
  case ast::LogicOp::OR:
    return createWrapperRef(
        ValManager.createValue<IntegerVal>(*Lhs || *Rhs, Type));
    break;
  case ast::LogicOp::GREATER:
    return createWrapperRef(
        ValManager.createValue<IntegerVal>(*Lhs > *Rhs, Type));
    break;
  case ast::LogicOp::GREATER_EQ:
    return createWrapperRef(
        ValManager.createValue<IntegerVal>(*Lhs >= *Rhs, Type));
    break;
  case ast::LogicOp::EQ:
    return createWrapperRef(
        ValManager.createValue<IntegerVal>(*Lhs == *Rhs, Type));
    break;
  case ast::LogicOp::NEQ:
    return createWrapperRef(
        ValManager.createValue<IntegerVal>(*Lhs != *Rhs, Type));
    break;
  default:
    llvm_unreachable("Unsupported logic operator");
  }
}

ResultTy InterpreterBase::performUnaryOperation(ast::UnOp Op, IntegerVal *Value,
                                                IntegerTy *Type) {
  assert(Value);
  assert(Type);
  switch (Op) {
  case ast::UnOp::PLUS:
    return createWrapperRef(Value);
    break;
  case ast::UnOp::MINUS:
    return createWrapperRef(ValManager.createValue<IntegerVal>(-*Value, Type));
    break;
  case ast::UnOp::NEGATE:
    return createWrapperRef(ValManager.createValue<IntegerVal>(!*Value, Type));
    break;
  default:
    llvm_unreachable("Unsupported unary operator");
  }
}

ResultTy InterpreterBase::performArithmeticOperation(ast::CalcOp Op,
                                                     IntegerVal *Lhs,
                                                     IntegerVal *Rhs,
                                                     IntegerTy *Type,
                                                     yy::location Loc) {
  assert(Lhs);
  assert(Rhs);
  assert(Type);
  switch (Op) {
  case ast::CalcOp::ADD:
    return createWrapperRef(
        ValManager.createValue<IntegerVal>(*Lhs + *Rhs, Type));
    break;
  case ast::CalcOp::SUB:
    return createWrapperRef(
        ValManager.createValue<IntegerVal>(*Lhs - *Rhs, Type));
    break;
  case ast::CalcOp::MUL:
    return createWrapperRef(
        ValManager.createValue<IntegerVal>(*Lhs * *Rhs, Type));
    break;
  case ast::CalcOp::PERCENT:
    return createWrapperRef(
        ValManager.createValue<IntegerVal>(*Lhs % *Rhs, Type));
    break;
  case ast::CalcOp::DIV:
    if (int check = *Rhs; check) {
      return createWrapperRef(
          ValManager.createValue<IntegerVal>(*Lhs / check, Type));
    } else {
      std::ostringstream Str;
      Str << Loc;
      paracl::fatal(llvm::formatv("{0}, trying to divide by 0", Str.str()));
    }
    break;
  default:
    llvm_unreachable("Unsupported calculation operator");
  }
}

ResultTy Interpreter::visit(ast::root_statement_block *StmBlock) {
  return visit(static_cast<ast::statement_block *>(StmBlock));
}

ResultTy Interpreter::visit(ast::statement_block *StmBlock) {
  auto &StmAccept = acceptStatementBlock(StmBlock);
  freeResources(StmBlock);
  return StmAccept;
}

ResultTy Interpreter::visit(ast::calc_expression *CalcExp) {
  auto *Lhs = acceptASTNode(CalcExp->left()).getAs<IntegerVal>();
  auto *Rhs = acceptASTNode(CalcExp->right()).getAs<IntegerVal>();
  auto *Type = Lhs->getType();
  assert(Type->isInt32Ty());

  return performArithmeticOperation(CalcExp->type(), Lhs, Rhs, Type,
                                    CalcExp->location());
}

ResultTy Interpreter::visit(ast::un_operator *UnOp) {
  auto *Value = acceptASTNode(UnOp->arg()).getAs<IntegerVal>();
  auto *Type = Value->getType();
  assert(Type->isInt32Ty());

  return performUnaryOperation(UnOp->type(), Value, Type);
}

ResultTy Interpreter::visit(ast::logic_expression *LogExp) {
  auto *Lhs = acceptASTNode(LogExp->left()).getAs<IntegerVal>();
  auto *Type = Lhs->getType();
  assert(Type->isInt32Ty());
  assert(Lhs);
  if (LogExp->type() == ast::LogicOp::AND && !*Lhs)
    return createWrapperRef(ValManager.createValue<IntegerVal>(0, Type));
  if (LogExp->type() == ast::LogicOp::OR && *Lhs)
    return createWrapperRef(ValManager.createValue<IntegerVal>(1, Type));
  auto *Rhs = acceptASTNode(LogExp->right()).getAs<IntegerVal>();
  return performLogicalOperation(LogExp->type(), Lhs, Rhs, Type);
}

ResultTy Interpreter::visit(ast::number *Num) {
  return createWrapperRef(ValManager.createValue<IntegerVal>(
      Num->get_value(), SymTbl.createType(TypeID::Int32)));
}

ResultTy Interpreter::visit(ast::variable *Var) {
  auto EntityKey = Var->entityKey();
  [[maybe_unused]] auto *Ty = SymTbl.getTypeFor(EntityKey);
  assert(Ty);
  return createWrapperRef(
      ValManager.getValueFor(SymTbl.getDeclKeyFor(EntityKey)));
}

ResultTy Interpreter::visit(ast::if_operator *If) {
  auto *CondVal = acceptASTNode(If->condition()).getAs<IntegerVal>();
  if (CondVal->getValue())
    return acceptASTNode(If->body());
  if (If->else_block())
    return acceptASTNode(If->else_block());
  return createWrapperRef();
}

ResultTy Interpreter::visit(ast::while_operator *While) {
  auto *CondVal = acceptASTNode(While->condition()).getAs<IntegerVal>();
  assert(CondVal);
  while (CondVal->getValue()) {
    acceptASTNode(While->body());
    CondVal = acceptASTNode(While->condition()).getAs<IntegerVal>();
  }
  return createWrapperRef();
}

ResultTy Interpreter::visit(ast::read_expression *ReadExp) {
  int Tmp = 0;
  input_stream_ >> Tmp;
  if (input_stream_.fail()) {
    std::ostringstream LocationStr;
    LocationStr << ReadExp->location();
    paracl::fatal(llvm::formatv("{0}: Non-integer data was transmitted",
                                LocationStr.str()));
  }

  return createWrapperRef(ValManager.createValue<IntegerVal>(
      Tmp, SymTbl.createType(TypeID::Int32)));
}

ResultTy Interpreter::visit(ast::print_function *Print) {
  auto *Val = acceptASTNode(Print->get()).get();
  assert(Val);
  Val->print(output_stream_);
  return createWrapperRef(Val);
}

ResultTy Interpreter::visit(ast::assignment *Assign) {
  auto *IdentExp = acceptASTNode(Assign->getIdentExp()).get();
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
      return createWrapperRef(IdentExp);
    }
    if (IdentType->isInt32Ty())
      return createWrapperRef(ValManager.createValueFor<IntegerVal>(
          DeclKey, static_cast<IntegerVal *>(IdentExp)->getValue(),
          SymTbl.createType(TypeID::Int32)));
    return createWrapperRef();
  }
  auto *Value =
      ValManager.getValueFor<IntegerVal>(SymTbl.getDeclKeyFor(EntityKey));
  assert(Value);
  Value->setValue(static_cast<IntegerVal *>(IdentExp));
  return createWrapperRef(IdentExp);
}

ResultTy Interpreter::visit(ast::ArrayHolder *ArrStore) {
  auto *Arr = ArrStore->get();
  assert(Arr);
  return acceptASTNode(Arr);
}

ResultTy Interpreter::visit(ast::PresetArray *PresetArr) {
  llvm::SmallVector<PCLValue *> PresetValues;
  PresetValues.reserve(PresetArr->size());
  for (auto *CurrExp : *PresetArr) {
    PresetValues.push_back(acceptASTNode(CurrExp).get());
  }

  auto *ArrVal = ValManager.createValue<PresetArrayVal>(
      PresetValues.begin(), PresetValues.end(),
      static_cast<ArrayTy *>(SymTbl.createType(TypeID::PresetArray)));
  addResourceForFree(ArrVal, PresetArr->scope());
  return createWrapperRef(ArrVal);
}

ResultTy Interpreter::visit(ast::ArrayAccess *ArrAccess) {
  auto AccessSize = ArrAccess->getSize();
  auto *CurrArr = ValManager.getValueFor<ArrayBase>(
      SymTbl.getDeclKeyFor(ArrAccess->entityKey()));
  assert(CurrArr);
  int CurrID = 0;
  for (unsigned ArrID = 1; auto RankID : *ArrAccess) {
    auto *RankVal = acceptASTNode(RankID).getAs<IntegerVal>();
    assert(RankVal);
    CurrID = RankVal->getValue();
    if (ArrID != AccessSize)
      CurrArr = static_cast<ArrayBase *>((*CurrArr)[CurrID]);
    ArrID++;
  }
  assert(CurrArr);
  return createWrapperRef((*CurrArr)[CurrID]);
}

ResultTy Interpreter::visit(ast::UniformArray *UnifArr) {
  auto *InitExpr = acceptASTNode(UnifArr->getInitExpr()).get();
  auto *Size = acceptASTNode(UnifArr->getSize()).getAs<IntegerVal>();

  assert(InitExpr && Size);
  auto *ArrVal = ValManager.createValue<UniformArrayVal>(
      InitExpr, *Size,
      static_cast<ArrayTy *>(SymTbl.createType(TypeID::UniformArray)));
  addResourceForFree(ArrVal, UnifArr->scope());
  return createWrapperRef(ArrVal);
}

ResultTy Interpreter::visit(ast::ArrayAccessAssignment *Arr) {
  auto *LValue = acceptASTNode(Arr->getArrayAccess()).getAs<IntegerVal>();
  auto *IdentExp = acceptASTNode(Arr->getIdentExp()).getAs<IntegerVal>();

  assert(LValue && IdentExp);
  LValue->setValue(IdentExp);
  return createWrapperRef(IdentExp);
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

ResultTy InterpreterBase::acceptStatementBlock(ast::statement_block *StmBlock) {
  for (auto &&statement : *StmBlock) {
    statement->accept(this);
  }
  return createWrapperRef();
}

void Interpreter::addResourceForFree(PCLValue *ValToFree,
                                     ast::statement_block *ScopeToFree) {
  assert(ScopeToFree);
  ResourceHandleMap[ScopeToFree].push_back(ValToFree);
}

} // namespace paracl
