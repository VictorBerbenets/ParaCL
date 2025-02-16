#include "codegen_visitor.hpp"

namespace paracl {
  
void CodeGenVisitor::visit(ast::definition *Def) {
}
void CodeGenVisitor::visit(ast::statement_block *stm) override {

}

void CodeGenVisitor::visit(ast::calc_expression *stm) {

}

void CodeGenVisitor::visit(ast::logic_expression *stm) {

}

void CodeGenVisitor::visit(ast::un_operator *stm)      {

}

void CodeGenVisitor::visit(ast::number *stm)           {

}

void CodeGenVisitor::visit(ast::variable *stm)         {

}

void CodeGenVisitor::visit(ast::assignment *stm)       {

}

void CodeGenVisitor::visit(ast::if_operator *stm)      {

}

void CodeGenVisitor::visit(ast::while_operator *stm)   {

}

void CodeGenVisitor::visit(ast::read_expression *stm)  {

}

void CodeGenVisitor::visit(ast::print_function *stm)   {

}


} // paracl
