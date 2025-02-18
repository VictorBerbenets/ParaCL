#include "codegen_visitor.hpp"
#include "ast_includes.hpp"

namespace paracl {

CodeGenVisitor::CodeGenVisitor(llvm::StringRef OutputFileName): Output(OutputFileName), CodeGen(Output) {}

void CodeGenVisitor::visit(ast::root_statement_block *RootBlock) {
  
}

void CodeGenVisitor::visit(ast::statement_block *StmBlock) {}

void CodeGenVisitor::visit(ast::definition *Def) {}

void CodeGenVisitor::visit(ast::calc_expression *CalcExpr) {}

void CodeGenVisitor::visit(ast::logic_expression *LogicExpr) {}

void CodeGenVisitor::visit(ast::un_operator *UnOper) {}

void CodeGenVisitor::visit(ast::number *Num) {}

void CodeGenVisitor::visit(ast::variable *Var) {}

void CodeGenVisitor::visit(ast::assignment *Assign) {}

void CodeGenVisitor::visit(ast::if_operator *stm) {}

void CodeGenVisitor::visit(ast::while_operator *stm) {}

void CodeGenVisitor::visit(ast::read_expression *stm) {}

void CodeGenVisitor::visit(ast::print_function *stm) {}

void CodeGenVisitor::generateIRCode(ast::root_statement_block *RootBlock) {
  RootBlock->accept(this);
}

} // namespace paracl
