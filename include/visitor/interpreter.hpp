#include <fstream>

#include "visitor.hpp"

namespace paracl {

class interpreter: visitor {
 public:
  interpreter(std::istream &input, std::ostream &output)
      : input_stream_ {input},
        output_stream_ {output} {}

  void visit_interpret(ast::calc_expression *stm)  override;
  void visit_interpret(ast::un_operator *stm)      override;
  void visit_interpret(ast::logic_expression *stm) override;
  void visit_interpret(ast::number *stm)           override;
  void visit_interpret(ast::variable *stm)         override;
  void visit_interpret(ast::assignment *stm)       override;
  void visit_interpret(ast::read_expression *stm)  override;
  void visit_interpret(ast::statement_block *stm)  override;
  void visit_interpret(ast::if_operator *stm)      override;
  void visit_interpret(ast::while_operator *stm)   override;
  void visit_interpret(ast::print_function *stm)   override;

  void run_program(ast::statement_block *root) {
    visit_interpret(root);
  }

 private:
  std::istream &input_stream_;
  std::ostream &output_stream_;
};

} // <--- namespace paracl
