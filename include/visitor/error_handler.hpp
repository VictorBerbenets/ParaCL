#pragma once

#include <string>
#include <vector>

#include "interpreter.hpp"
#include "location.hh"

namespace paracl {

class ErrorHandler : public InterpreterBase {
  using StringErrType = std::string;
  using ErrorType = std::pair<StringErrType, yy::location>;
  using TypePtr = PCLType *;

public:
  void visit(ast::root_statement_block *StmBlock) override;

  void visit(ast::statement_block *StmBlock) override;

  void visit(ast::calc_expression *CalcExp) override;

  void visit(ast::logic_expression *LogExp) override;

  void visit(ast::un_operator *UnOp) override;

  void visit(ast::number *Num) override;

  void visit(ast::assignment *Assign) override;

  void visit(ast::variable *Assign) override;

  void visit(ast::if_operator *If) override;

  void visit(ast::while_operator *While) override;

  void visit(ast::read_expression * /*unused*/) override;

  void visit(ast::print_function * /*unused*/) override;

  void visit(ast::ArrayHolder *ArrStore) override;

  void visit(ast::PresetArray *PresetArr) override;

  void visit(ast::UniformArray *UnifArr) override;

  void visit(ast::ArrayAccess *ArrAccess) override;

  void visit(ast::ArrayAccessAssignment *ArrAssign) override;

  void run(ast::statement_block *root);

  void print_errors(llvm::raw_ostream &Os, const std::string &FileName) const;

  [[nodiscard]] bool empty() const noexcept;
  unsigned size() const noexcept;

  auto begin() noexcept;
  auto end() noexcept;
  auto begin() const noexcept;
  auto end() const noexcept;

  void setTypeAndValue(TypePtr Ty, ValuePtr Val);

  std::pair<PCLType *, PCLValue *>
  getTypeAndValueAfterAccept(ast::statement *Stm);

private:
  StringErrType makeValidationMessage(const std::string &ErrMes,
                                      yy::location Loc) const;

  StringErrType makeValidationMessage(ErrorType Err,
                                      llvm::StringRef Diagnostics = "") const;

  unsigned computeArrayDimension(ArrayTy *Arr);

  std::vector<ErrorType> Errors;
  TypePtr CurrTy;
};

} // namespace paracl
