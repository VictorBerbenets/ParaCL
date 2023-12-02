#include "visitor.hpp"

namespace frontend {

class interpreter: visitor {
 public:
  void visit(ast::statement_block *stm)  override;
  void visit(ast::bin_operator *stm)     override;
  void visit(ast::un_operator *stm)      override;
  void visit(ast::logic_expression *stm) override;
  void visit(ast::number *stm)           override;
  void visit(ast::variable *stm)         override;
  void visit(ast::if_operator *stm)      override;
  void visit(ast::while_operator *stm)   override;
  void visit(ast::scan_function *stm)    override;
  void visit(ast::print_function *stm)   override;
  void visit(ast::assignment *stm)   override;

  void run_program(ast::statement_block *root) {
    visit(root);
  }
};

} // <--- namespace frontend
