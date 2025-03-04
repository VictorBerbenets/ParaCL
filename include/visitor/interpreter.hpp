#include <fstream>

#include "statement.hpp"
#include "visitor.hpp"
#include "visitor_tracker.hpp"

namespace paracl {

class interpreter : public VisitorTracker {
public:
  interpreter(std::istream &input, std::ostream &output)
      : input_stream_{input}, output_stream_{output} {}

  void visit(ast::ArrayAccessAssignment *Arr) override;

  void visit(ast::PresetArray *InitListArr) override;
  void visit(ast::ArrayAccess *ArrAccess) override;
  void visit(ast::UniformArray *Arr) override;

  void visit(ast::calc_expression *CalcExpr) override;
  void visit(ast::un_operator *UnOp) override;
  void visit(ast::logic_expression *LogExpr) override;
  void visit(ast::number *Num) override;
  void visit(ast::variable *Var) override;
  void visit(ast::assignment *Assign) override;
  void visit(ast::read_expression *ReadExpr) override;
  void visit(ast::statement_block *StmBlock) override;
  void visit(ast::if_operator *If) override;
  void visit(ast::while_operator *While) override;
  void visit(ast::print_function *Print) override;

private:
  std::istream &input_stream_;
  std::ostream &output_stream_;
};

} // namespace paracl
