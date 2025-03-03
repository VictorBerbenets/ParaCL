#pragma once

#include "llvm/Support/FormatVariadic.h"
#include "llvm/ADT/SmallVector.h"

#include <iostream>
#include <vector>

#include "ast_includes.hpp"
#include "location.hh"
#include "visitor_tracker.hpp"

namespace paracl {

class ErrorHandler : public VisitorTracker {
  using ErrorType = std::pair<const std::string, yy::location>;

public:
  void visit(ast::ArrayAccessAssignment *Arr) override {}
  void visit(ast::PresetArray *InitListArr) override {}
  void visit(ast::ArrayAccess *ArrAccess) override {}
  void visit(ast::UniformArray *Arr) override {}
  
  void visit(ast::root_statement_block *stm) override {
    for (auto &&statement : *stm) {
      statement->accept(this);
    }
  }

  void visit(ast::statement_block *stm) override {
    for (auto &&statement : *stm) {
      statement->accept(this);
    }
  }

  void visit(ast::calc_expression *CalcExpr) override {
    auto *Lhs = getValueAfterAccept<IntegerVal>(CalcExpr->left());
    auto *Rhs = getValueAfterAccept<IntegerVal>(CalcExpr->right());
    assert(Lhs);
    auto *Type = Lhs->getType();
    if (!Type->isInt32Ty())
      return ;

    switch (CalcExpr->type()) {
    case ast::CalcOp::ADD:
      set_value(ValManager.createValue<IntegerVal>(*Lhs + *Rhs, Type));
      break;
    case ast::CalcOp::SUB:
      set_value(ValManager.createValue<IntegerVal>(*Lhs - *Rhs, Type));
      break;
    case ast::CalcOp::MUL:
      set_value(ValManager.createValue<IntegerVal>(*Lhs * *Rhs, Type));
      break;
    case ast::CalcOp::PERCENT:
      set_value(ValManager.createValue<IntegerVal>(*Lhs % *Rhs, Type));
      break;
    case ast::CalcOp::DIV:
      if (int check = *Rhs; check) {
        set_value(ValManager.createValue<IntegerVal>(*Lhs / check, Type));
      } else {
        throw std::runtime_error{"trying to divide by 0"};
      }
      break;
    default:
      throw std::logic_error{"unrecognized logic type"};
    }
  }

  void visit(ast::logic_expression *LogExpr) override {
    auto *Lhs = getValueAfterAccept<IntegerVal>(LogExpr->left());
    auto *Rhs = getValueAfterAccept<IntegerVal>(LogExpr->right());
    auto *Type = Lhs->getType();

    switch (LogExpr->type()) {
    case ast::LogicOp::LESS:
      set_value(ValManager.createValue<IntegerVal>(*Lhs < *Rhs, Type));
      break;
    case ast::LogicOp::LESS_EQ:
      set_value(ValManager.createValue<IntegerVal>(*Lhs <= *Rhs, Type));
      break;
    case ast::LogicOp::LOGIC_AND:
      set_value(ValManager.createValue<IntegerVal>(*Lhs && *Rhs, Type));
      break;
    case ast::LogicOp::LOGIC_OR:
      set_value(ValManager.createValue<IntegerVal>(*Lhs || *Rhs, Type));
      break;
    case ast::LogicOp::GREATER:
      set_value(ValManager.createValue<IntegerVal>(*Lhs > *Rhs, Type));
      break;
    case ast::LogicOp::GREATER_EQ:
      set_value(ValManager.createValue<IntegerVal>(*Lhs >= *Rhs, Type));
      break;
    case ast::LogicOp::EQ:
      set_value(ValManager.createValue<IntegerVal>(*Lhs == *Rhs, Type));
      break;
    case ast::LogicOp::NEQ:
      set_value(ValManager.createValue<IntegerVal>(*Lhs != *Rhs, Type));
      break;
    default:
      throw std::logic_error{"unrecognized logic type"};
    }
  }

  void visit(ast::un_operator *UnOp) override {
    auto *Value = getValueAfterAccept<IntegerVal>(UnOp->arg());
    auto *Type = Value->getType();
    if (!Type->isInt32Ty())
      return ;

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

  void visit(ast::number * /*unused*/) override {}

  void visit(ast::variable *Var) override {
    auto *CurrScope = Var->scope();
    auto Name = Var->name();
    if (!SymTbl.isDefined({Name, CurrScope})) {
      Errors.push_back(
          {llvm::formatv("{0} was not declared in this scope", Name), Var->location()});
      setTypeAndValue(nullptr, nullptr);
    } else {
      auto *Type = SymTbl.getTypeFor(Name, CurrScope);
      assert(Type);
      auto *DeclScope = SymTbl.getDeclScopeFor(Name, CurrScope);
      auto *Value = ValManager.getValueFor({Name, DeclScope});
      setTypeAndValue(Type, Value);
    }
  }

  void visit(ast::assignment *Assign) override {
    auto VarName = Assign->name();
    SymTbl.tryDefine(VarName, Assign->scope(), SymTbl.getType(Assign->getID()));
    auto *DeclScope = SymTbl.getDeclScopeFor(VarName, Assign->scope());
    auto [IdentType, _] = getTypeAndValueAfterAccept(Assign->getIdentExp());
    auto [LValueType, _] = getTypeAndValueAfterAccept(Assign->getLValue());

    if (!IdentType || !LValueType)
      Errors.emplace_back(llvm::formatv("Expression is not assignable. Couldn't deduce the types for initializing the {0} variable", VarName), Assign->location());
    else if (IdentType && LValueType && *IdentType != *LValueType)
      Errors.push_back({llvm::formatv("Expression is not assignable. Couldn't convert {0} to {1}", IdentType->getName(), LValueType->getName()), Assign->location()});  
    else 
      ValManager.linkValueWithName({VarName, DeclScope}, nullptr);
  }

  void visit(ast::if_operator *stm) override {
    stm->condition()->accept(this);
    stm->body()->accept(this);
  }

  void visit(ast::while_operator *While) override {
    auto *CondVal = getValueAfterAccept<IntegerVal>(While->condition());
    assert(CondVal);
    while (CondVal->getValue()) {
      While->body()->accept(this);
      CondVal = getValueAfterAccept<IntegerVal>(While->condition());
    }
  }

  void visit(ast::read_expression * /*unused*/) override {
    set_value(
        ValManager.createValue<IntegerVal>(0, SymTbl.getType(TypeID::Int32)));
  }

  void visit(ast::print_function *stm) override {
    auto exp = stm->get();
    exp->accept(this);
  }

  void run(ast::statement_block *root) { visit(root); }

  void print_errors(std::ostream &stream) const {
    for (auto &&err : Errors) {
      stream << err.second << " : " << err.first << std::endl;
    }
  }

  [[nodiscard]] bool empty() const noexcept { return Errors.empty(); }
  unsigned size() const noexcept { return Errors.size(); }

  auto begin() noexcept { return Errors.begin(); }
  auto end() noexcept { return Errors.end(); }
  auto begin() const noexcept { return Errors.begin(); }
  auto end() const noexcept { return Errors.end(); }
      
  void setTypeAndValue(PCLType *Ty, PCLValue* Val) {
    CurrTy = Ty;
    CurrValue = Val;
  }

  std::pair<PCLType *, PCLValue*> getTypeAndValueAfterAccept(ast::statement *Stm) {
    Stm->accept(this);
    return {CurrTy, CurrValue};
  }

private:
  std::vector<ErrorType> Errors;
  PCLType *CurrTy;
};

} // namespace paracl
