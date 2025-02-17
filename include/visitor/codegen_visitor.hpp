#pragma once

#include <llvm/ADT/StringRef.h>

#include "visitor.hpp"

namespace paracl {

class CodeGenVisitor : public base_visitor {
public:
  virtual void visit(ast::root_statement_block *stm);
  virtual void visit(ast::definition *stm);

  void visit(ast::calc_expression *stm) override;
  void visit(ast::un_operator *stm) override;
  void visit(ast::logic_expression *stm) override;
  void visit(ast::number *stm) override;
  void visit(ast::variable *stm) override;
  void visit(ast::assignment *stm) override;
  void visit(ast::read_expression *stm) override;
  void visit(ast::statement_block *stm) override;
  void visit(ast::if_operator *stm) override;
  void visit(ast::while_operator *stm) override;
  void visit(ast::print_function *stm) override;

  void generateIRCode(ast::root_statement_block *RootBlock,
                      llvm::StringRef Output);
};

} // namespace paracl
