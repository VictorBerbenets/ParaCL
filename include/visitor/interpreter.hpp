#include <fstream>

#include "symbol_table.hpp"
#include "statement.hpp"
#include "visitor.hpp"

namespace paracl {

class interpreter : public base_visitor {
  using value_type = PCLValue *;
  using TypeID = SymTable::TypeID;

public:
  interpreter(SymTable &SymTbl, ValueManager &ValManager, std::istream &input,
              std::ostream &output)
      : SymTbl(SymTbl), ValManager(ValManager), input_stream_{input},
        output_stream_{output} {}

  void virtual visit(ast::root_statement_block *StmBlock);
  void visit(ast::ArrayAccessAssignment *Arr) override;

  void visit(ast::InitListArray *InitListArr) override;
  void visit(ast::ArrayAccess *ArrAccess) override;
  void visit(ast::UndefVar *UndVar) override;
  void visit(ast::Array *Arr) override;

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

  void run_program(ast::root_statement_block *StmBlock) { visit(StmBlock); }

  value_type get_value() const noexcept { return curr_value_; }

  void set_value(value_type val) noexcept { curr_value_ = val; }

  template <DerivedFromPCLValue ValueType = PCLValue>
  ValueType *getValueAfterAccept(ast::statement *Stm) {
    Stm->accept(this);
    assert(curr_value_);
    return static_cast<ValueType*>(curr_value_);
  }

private:
  SymTable &SymTbl;
  ValueManager &ValManager;
  std::istream &input_stream_;
  std::ostream &output_stream_;

  value_type curr_value_ = nullptr;
};

} // namespace paracl
