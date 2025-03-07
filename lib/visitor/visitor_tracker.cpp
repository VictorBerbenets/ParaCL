#include <llvm/Support/FormatVariadic.h>

#include <sstream>

#include "visitor_tracker.hpp"

namespace paracl {

VisitorTracker::ValueTypePtr
VisitorTracker::performLogicalOperation(ast::LogicOp Op, IntegerVal *Lhs,
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

VisitorTracker::ValueTypePtr
VisitorTracker::performUnaryOperation(ast::UnOp Op, IntegerVal *Value,
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

VisitorTracker::ValueTypePtr
VisitorTracker::performArithmeticOperation(ast::CalcOp Op, IntegerVal *Lhs,
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

} // namespace paracl
