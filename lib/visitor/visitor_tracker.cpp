
#include "visitor_tracker.hpp"

namespace paracl {

VisitorTracker::ValueTypePtr VisitorTracker::performLogicalOperation(ast::LogicOp Op, IntegerVal *Lhs, IntegerVal *Rhs, IntegerTy *Type) {
    switch (Op) {
    case ast::LogicOp::LESS:
      return ValManager.createValue<IntegerVal>(*Lhs < *Rhs, Type);
    case ast::LogicOp::LESS_EQ:
      return ValManager.createValue<IntegerVal>(*Lhs <= *Rhs, Type);
      break;
    case ast::LogicOp::LOGIC_AND:
      return ValManager.createValue<IntegerVal>(*Lhs && *Rhs, Type);
      break;
    case ast::LogicOp::LOGIC_OR:
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

} // namespace paracl
