#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/Support/FormatVariadic.h>

#include <sstream>

#include "ast_includes.hpp"
#include "error_handler.hpp"

namespace paracl {

void ErrorHandler::visit(ast::root_statement_block *StmBlock) {
  ErrorHandler::visit(static_cast<ast::statement_block *>(StmBlock));
}

void ErrorHandler::visit(ast::statement_block *StmBlock) {
  acceptStatementBlock(StmBlock);
}

void ErrorHandler::visit(ast::calc_expression *CalcExp) {
  auto [LhsTy, LhsVal] = getTypeAndValueAfterAccept(CalcExp->left());
  auto [RhsTy, RhsVal] = getTypeAndValueAfterAccept(CalcExp->right());
  if (!LhsTy || !RhsTy) {
    Errors.emplace_back("expression is not computable. Couldn't deduce the "
                        "types for arithmetic operation.",
                        CalcExp->location());
  } else {
    if (*LhsTy != *RhsTy)
      Errors.push_back(
          {llvm::formatv("expression is not computable. Couldn't compute "
                         "values with types '{0}' and '{1}'",
                         LhsTy->getName(), RhsTy->getName()),
           CalcExp->location()});
    else if (!LhsTy->isInt32Ty())
      Errors.push_back(
          {llvm::formatv("expression is not computable. Couldn't compute "
                         "values of '{0}' type.",
                         LhsTy->getName()),
           CalcExp->location()});
    else if (LhsVal && RhsVal)
      setTypeAndValue(
          LhsTy, performArithmeticOperation(
                     CalcExp->type(), static_cast<IntegerVal *>(LhsVal),
                     static_cast<IntegerVal *>(RhsVal),
                     static_cast<IntegerTy *>(LhsTy), CalcExp->location()));
    else
      setTypeAndValue(LhsTy, nullptr);
  }
}

void ErrorHandler::visit(ast::logic_expression *LogExp) {
  auto [LhsTy, LhsVal] = getTypeAndValueAfterAccept(LogExp->left());
  auto [RhsTy, RhsVal] = getTypeAndValueAfterAccept(LogExp->right());
  if (!LhsTy || !RhsTy) {
    Errors.emplace_back("expression is not comparable. Couldn't deduce the "
                        "types for logic comparison.",
                        LogExp->location());
  } else {
    if (*LhsTy != *RhsTy)
      Errors.push_back({llvm::formatv("expression is not comparable. "
                                      "Couldn't compare '{0}' and '{1}'",
                                      LhsTy->getName(), RhsTy->getName()),
                        LogExp->location()});
    else if (!LhsTy->isInt32Ty())
      Errors.push_back(
          {llvm::formatv("expression is not comparable. Couldn't compare "
                         "values of '{0}' type.",
                         LhsTy->getName()),
           LogExp->location()});
    else if (LhsVal && RhsVal)
      setTypeAndValue(LhsTy,
                      performLogicalOperation(LogExp->type(),
                                              static_cast<IntegerVal *>(LhsVal),
                                              static_cast<IntegerVal *>(RhsVal),
                                              static_cast<IntegerTy *>(LhsTy)));
    else
      setTypeAndValue(LhsTy, nullptr);
  }
}

void ErrorHandler::visit(ast::un_operator *UnOp) {
  auto [Type, Value] = getTypeAndValueAfterAccept(UnOp->arg());
  if (!Type) {
    Errors.emplace_back(
        "couldn't calculate the unary operation. Type is unknown",
        UnOp->location());
  } else {
    if (!Type->isInt32Ty())
      Errors.push_back(
          {llvm::formatv("couldn't apply a unary operation to the '{0}' type.",
                         Type->getName()),
           UnOp->location()});
    else if (Value)
      setTypeAndValue(Type, performUnaryOperation(
                                UnOp->type(), static_cast<IntegerVal *>(Value),
                                static_cast<IntegerTy *>(Type)));
    else
      setTypeAndValue(Type, nullptr);
  }
}

void ErrorHandler::visit(ast::number *Num) {
  auto *Type = SymTbl.createType(TypeID::Int32);
  setTypeAndValue(Type,
                  ValManager.createValue<IntegerVal>(Num->get_value(), Type));
}

void ErrorHandler::visit(ast::variable *Var) {
  auto EntityKey = Var->entityKey();
  if (!SymTbl.isDefined(EntityKey)) {
    Errors.push_back(
        {llvm::formatv("{0} was not declared in this scope", Var->name()),
         Var->location()});
    setTypeAndValue(nullptr, nullptr);
  } else {
    auto *Type = SymTbl.getTypeFor(EntityKey);
    assert(Type);
    auto *Value = ValManager.getValueFor(SymTbl.getDeclKeyFor(EntityKey));
    setTypeAndValue(Type, Value);
  }
}

void ErrorHandler::visit(ast::assignment *Assign) {
  static constexpr llvm::StringRef ErrDesc = "expression is not assignable";

  auto [IdentType, IdentVal] =
      getTypeAndValueAfterAccept(Assign->getIdentExp());
  auto EntityKey = Assign->entityKey();
  if (!SymTbl.isDefined(EntityKey)) {
    assert(IdentType);
    if (SymTbl.containsKeyWithType(IdentType) && IdentType->isArrayTy()) {
      Errors.emplace_back(
          llvm::formatv("{0}: arrays cannot be copy constructed", ErrDesc),
          Assign->location());
      return;
    }
    [[maybe_unused]] auto IsDefined = SymTbl.tryDefine(EntityKey, IdentType);
    assert(IsDefined);
  } else if (IdentType && IdentType->isArrayTy()) {
    Errors.emplace_back(
        llvm::formatv("{0}: arrays cannot be assigned", ErrDesc),
        Assign->location());
  }

  auto [LValueType, LVal] = getTypeAndValueAfterAccept(Assign->getLValue());
  if (!IdentType || !LValueType)
    Errors.emplace_back(llvm::formatv("{0}: couldn't deduce the "
                                      "types for initializing the {1} variable",
                                      ErrDesc, Assign->name()),
                        Assign->location());
  else if (IdentType && LValueType && *IdentType != *LValueType)
    Errors.push_back(
        {llvm::formatv("{0}: couldn't convert '{1}' to '{2}'", ErrDesc,
                       IdentType->getName(), LValueType->getName()),
         Assign->location()});
  else
    ValManager.linkValueWithName(SymTbl.getDeclKeyFor(EntityKey), nullptr);
}

void ErrorHandler::visit(ast::if_operator *If) {
  auto [Type, Value] = getTypeAndValueAfterAccept(If->condition());
  if (!Type)
    Errors.emplace_back("couldn't calculate the conditional expression for "
                        "the if statement. Type is unknown",
                        If->location());
  else if (!Type->isInt32Ty())
    Errors.emplace_back(
        llvm::formatv("couldn't calculate the conditional expression of the "
                      "{0} type for the if statement. Only integer types are "
                      "expected in conditional expressions.",
                      Type->getName()),
        If->location());

  If->body()->accept(this);
}

void ErrorHandler::visit(ast::while_operator *While) {
  auto [Type, Value] = getTypeAndValueAfterAccept(While->condition());
  if (!Type)
    Errors.emplace_back("couldn't calculate the conditional expression for "
                        "the while statement. Type is unknown",
                        While->location());
  else if (!Type->isInt32Ty())
    Errors.emplace_back(
        llvm::formatv("couldn't calculate the conditional expression of the "
                      "{0} type for the while statement. Only integer types "
                      "are expected in conditional expressions.",
                      Type->getName()),
        While->location());

  While->body()->accept(this);
}

void ErrorHandler::visit(ast::read_expression * /*unused*/) {
  // Pass nullptr as Value* because we handle only 'compile time' cases
  setTypeAndValue(SymTbl.createType(TypeID::Int32), nullptr);
}

void ErrorHandler::visit(ast::print_function *Print) {
  auto [Type, _] = getTypeAndValueAfterAccept(Print->get());
  if (Type && !Type->isInt32Ty())
    Errors.emplace_back(llvm::formatv("invalid argument. The print function "
                                      "requires integer argument, got '{0}'",
                                      Type->getName()),
                        Print->location());
}

void ErrorHandler::visit(ast::ArrayHolder *ArrStore) {
  auto *Arr = ArrStore->get();
  assert(Arr);
  Arr->accept(this);
}

void ErrorHandler::visit(ast::PresetArray *PresetArr) {
  llvm::SmallVector<StringErrType> InvalidArrArgs;
  std::optional<unsigned> ArrSz = 0;
  auto IncreaseIfNotNullopt = [&](auto Sz) {
    if (ArrSz.has_value())
      ArrSz.value() += Sz;
  };

  for (auto *CurrElem : *PresetArr) {
    auto [Type, _] = getTypeAndValueAfterAccept(CurrElem);
    if (!Type) {
      InvalidArrArgs.emplace_back(makeValidationMessage(
          "couldn't deduce the type for passed element", CurrElem->location()));
    } else if (Type->isArrayTy()) {
      auto *ArrType = static_cast<ArrayTy *>(Type);
      if (auto SizeOpt = ArrType->getSize(); SizeOpt.has_value())
        IncreaseIfNotNullopt(SizeOpt.value());
      else
        InvalidArrArgs.emplace_back(
            makeValidationMessage("array must be "
                                  "able to output the size before execution, "
                                  "repeat size is not constant",
                                  CurrElem->location()));

      auto *ContainedType = ArrType->getContainedType();
      assert(ContainedType);
      if (ContainedType->isArrayTy()) {
        InvalidArrArgs.emplace_back(makeValidationMessage(
            "it is forbidden to pass arrays of more than one dimension",
            CurrElem->location()));
        ArrSz = std::nullopt;
      }
    } else {
      IncreaseIfNotNullopt(1);
    }
  }
  if (!InvalidArrArgs.empty()) {
    StringErrType TopErrorMes("couldn't create preset array:\n");
    Errors.emplace_back(TopErrorMes + llvm::join(InvalidArrArgs, "\n"),
                        PresetArr->location());
  }
  auto *ArrType =
      static_cast<ArrayTy *>(SymTbl.createType(TypeID::PresetArray));
  ArrType->setContainedType(SymTbl.createType(TypeID::Int32));
  if (ArrSz.has_value())
    ArrType->setSize(ArrSz.value());
  setTypeAndValue(ArrType, nullptr);
}

void ErrorHandler::visit(ast::UniformArray *UnifArr) {
  auto [ContainType, _] = getTypeAndValueAfterAccept(UnifArr->getInitExpr());
  auto [SizeType, SizeVal] = getTypeAndValueAfterAccept(UnifArr->getSize());
  auto *ArrType =
      static_cast<ArrayTy *>(SymTbl.createType(TypeID::UniformArray));
  if (SizeType) {
    if (!SizeType->isInt32Ty())
      Errors.emplace_back(llvm::formatv("couldn't convert '{0} to integer type",
                                        SizeType->getName()),
                          UnifArr->location());
    else if (SizeVal)
      ArrType->setSize(static_cast<IntegerVal *>(SizeVal)->getValue());
  }
  ArrType->setContainedType(ContainType);
  setTypeAndValue(ArrType, nullptr);
}

void ErrorHandler::visit(ast::ArrayAccess *ArrAccess) {
  auto [Name, DeclScope] = SymTbl.getDeclKeyFor(ArrAccess->entityKey());
  if (!DeclScope) {
    Errors.emplace_back(
        llvm::formatv("use of undeclared identifier '{0}'", Name),
        ArrAccess->location());
    return;
  }

  auto *CurrTy = SymTbl.getTypeFor(Name, DeclScope);
  assert(CurrTy);
  if (!CurrTy || !CurrTy->isArrayTy() ||
      computeArrayDimension(static_cast<ArrayTy *>(CurrTy)) !=
          ArrAccess->getSize()) {
    Errors.emplace_back("subscripted value is not an array",
                        ArrAccess->location());
    setTypeAndValue(nullptr, nullptr);
    return;
  }
  for (auto *CurrArrTy = static_cast<ArrayTy *>(CurrTy);
       auto RankID : *ArrAccess) {
    auto [RankTy, RankVal] = getTypeAndValueAfterAccept(RankID);
    if (RankTy) {
      if (!RankTy->isInt32Ty()) {
        Errors.emplace_back(llvm::formatv("couldn't access the array '{0}', "
                                          "the index types must be integer",
                                          Name),
                            ArrAccess->location());
      } else if (RankVal) {
        auto *IntRankVal = static_cast<IntegerVal *>(RankVal);
        auto Index = IntRankVal->getValue();
        if (Index < 0)
          Errors.emplace_back(llvm::formatv("array index {0} is before the "
                                            "beginning of the array '{1}'",
                                            Index, Name),
                              ArrAccess->location());
        else if (auto ArrSzOpt = CurrArrTy->getSize();
                 ArrSzOpt.has_value() &&
                 ArrSzOpt.value() <= static_cast<unsigned>(Index))
          Errors.emplace_back(
              llvm::formatv("array index {0} is past the end of the array "
                            "(that has size {1})",
                            Index, ArrSzOpt.value()),
              ArrAccess->location());
      }
    }
    CurrArrTy = static_cast<ArrayTy *>(CurrArrTy->getContainedType());
  }
  setTypeAndValue(SymTbl.createType(TypeID::Int32), nullptr);
}

void ErrorHandler::visit(ast::ArrayAccessAssignment *ArrAssign) {
  if (!SymTbl.isDefined(ArrAssign->entityKey())) {
    Errors.emplace_back(
        llvm::formatv("use of undeclared identifier '{0}'", ArrAssign->name()),
        ArrAssign->location());
    setTypeAndValue(nullptr, nullptr);
  } else {
    auto [LhsTy, AccVal] =
        getTypeAndValueAfterAccept(ArrAssign->getArrayAccess());
    auto [RhsTy, IdentVal] =
        getTypeAndValueAfterAccept(ArrAssign->getIdentExp());
    if (RhsTy && RhsTy->isArrayTy())
      Errors.emplace_back(
          "expression is not assignable. Arrays cannot be assigned",
          ArrAssign->location());
    if (LhsTy && RhsTy && *LhsTy != *RhsTy) {
      Errors.push_back({llvm::formatv("expression is not assignable. "
                                      "Couldn't convert '{0}' to '{1}'",
                                      RhsTy->getName(), LhsTy->getName()),
                        ArrAssign->location()});
    }
  }
}

void ErrorHandler::run(ast::statement_block *root) { visit(root); }

void ErrorHandler::print_errors(llvm::raw_ostream &Os,
                                const std::string &FileName) const {
  for (auto &&Err : Errors) {
    Os << llvm::formatv("{0}:{1}\n", FileName,
                        makeValidationMessage(Err, "error"));
  }
}

[[nodiscard]] bool ErrorHandler::empty() const noexcept {
  return Errors.empty();
}
unsigned ErrorHandler::size() const noexcept { return Errors.size(); }

auto ErrorHandler::begin() noexcept { return Errors.begin(); }
auto ErrorHandler::end() noexcept { return Errors.end(); }
auto ErrorHandler::begin() const noexcept { return Errors.begin(); }
auto ErrorHandler::end() const noexcept { return Errors.end(); }

void ErrorHandler::setTypeAndValue(TypePtr Ty, ValuePtr Val) {
  CurrTy = Ty;
  CurrValue = Val;
}

std::pair<PCLType *, PCLValue *>
ErrorHandler::getTypeAndValueAfterAccept(ast::statement *Stm) {
  Stm->accept(this);
  return {CurrTy, CurrValue};
}

ErrorHandler::StringErrType
ErrorHandler::makeValidationMessage(const std::string &ErrMes,
                                    yy::location Loc) const {
  return makeValidationMessage({ErrMes, Loc});
}

ErrorHandler::StringErrType
ErrorHandler::makeValidationMessage(ErrorType Err,
                                    llvm::StringRef Diagnostics) const {
  std::stringstream ErrPos;
  ErrPos << Err.second;
  auto DiagnosticStr =
      (Diagnostics != "" ? std::string(Diagnostics).append(": ") : "");
  return llvm::formatv("{0}: {2}{1}", ErrPos.str(), Err.first, DiagnosticStr);
}

unsigned ErrorHandler::computeArrayDimension(ArrayTy *Arr) {
  assert(Arr);
  auto *ContainedType = Arr->getContainedType();
  if (!ContainedType || !ContainedType->isArrayTy()) {
    return 1;
  }
  unsigned Dim = 2;
  for (auto *CurrArray = static_cast<ArrayTy *>(ContainedType); CurrArray;) {
    if (auto *ContType = CurrArray->getContainedType(); ContType->isArrayTy()) {
      Dim++;
      CurrArray = static_cast<ArrayTy *>(ContType);
    } else {
      break;
    }
  }
  return Dim;
}

} // namespace paracl
