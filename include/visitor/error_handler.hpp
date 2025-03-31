#pragma once

#include <string>
#include <vector>

#include "interpreter.hpp"
#include "location.hh"

namespace paracl {

// When we perform syntactic analysis, it is useful to get the type and value as
// the result of the operation.
struct HandlerWrapper : public ValueWrapper {
  using Type = PCLType;
  using Value = PCLValue;
  using TypePtr = Type *;
  using ValuePtr = Value *;

  HandlerWrapper(TypePtr Ty, ValuePtr Val) : Ty(Ty), Val(Val) {}

  TypePtr Ty;
  ValuePtr Val;
};

class ErrorHandler : public InterpreterBase {
  using StringErrType = std::string;
  using ErrorType = std::pair<StringErrType, yy::location>;

public:
  using WrapperTy = HandlerWrapper;
  using ResultTy = WrapperTy &;

  ResultTy visit(ast::root_statement_block *StmBlock) override;
  ResultTy visit(ast::statement_block *StmBlock) override;
  ResultTy visit(ast::calc_expression *CalcExp) override;
  ResultTy visit(ast::logic_expression *LogExp) override;
  ResultTy visit(ast::un_operator *UnOp) override;
  ResultTy visit(ast::number *Num) override;
  ResultTy visit(ast::assignment *Assign) override;
  ResultTy visit(ast::variable *Assign) override;
  ResultTy visit(ast::if_operator *If) override;
  ResultTy visit(ast::while_operator *While) override;
  ResultTy visit(ast::read_expression *ReadExp) override;
  ResultTy visit(ast::print_function *PrintFunc) override;
  ResultTy visit(ast::ArrayHolder *ArrStore) override;
  ResultTy visit(ast::PresetArray *PresetArr) override;
  ResultTy visit(ast::UniformArray *UnifArr) override;
  ResultTy visit(ast::ArrayAccess *ArrAccess) override;
  ResultTy visit(ast::ArrayAccessAssignment *ArrAssign) override;

  void run(ast::statement_block *root);

  void print_errors(llvm::raw_ostream &Os, const std::string &FileName) const;

  [[nodiscard]] bool empty() const noexcept;
  unsigned size() const noexcept;

  auto begin() noexcept;
  auto end() noexcept;
  auto begin() const noexcept;
  auto end() const noexcept;

private:
  ResultTy acceptASTNode(ast::statement *Stm) override {
    return static_cast<ResultTy>(Stm->accept(this));
  }

  ResultTy createWrapperRef(HandlerWrapper::TypePtr Ty = nullptr,
                            HandlerWrapper::ValuePtr Val = nullptr) {
    return VisitorBase::createWrapperRef<WrapperTy>(Ty, Val);
  }

  StringErrType makeValidationMessage(const std::string &ErrMes,
                                      yy::location Loc) const;

  StringErrType makeValidationMessage(ErrorType Err,
                                      llvm::StringRef Diagnostics = "") const;

  bool isPrintableType(const PCLType &Type) const noexcept;

  unsigned computeArrayDimension(ArrayTy *Arr);

  std::vector<ErrorType> Errors;
};

} // namespace paracl
