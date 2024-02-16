#pragma once

#include <fstream>

#include "visitor.hpp"

namespace frontend {

class interpreter: visitor {
 public:
  interpreter(std::istream &input, std::ostream &output)
      : input_stream_ {input},
        output_stream_ {output} {}

  void visit(ast::statement_block *stm) override {
    for (auto&& statement : *stm) {
      statement->accept(this);
    }
  }

  void visit(ast::calc_expression *stm) override {
    stm->left()->accept(this);
    auto lhs = get_value();
    stm->right()->accept(this);
    auto rhs = get_value();

    switch(stm->type()) {
      case ast::CalcOp::ADD :
        set_value(lhs + rhs);
        break;
      case ast::CalcOp::SUB :
        set_value(lhs - rhs);
        break;
      case ast::CalcOp::MUL :
        set_value(lhs * rhs);
        break;
      case ast::CalcOp::PERCENT :
        set_value(lhs % rhs);
        break;
      case ast::CalcOp::DIV :
        if (auto check = rhs; check) {
          set_value(lhs / check);
        } else {
          throw std::runtime_error{"trying to divide by 0"};
        }
        break;
      default: throw std::logic_error{"unrecognized logic type"};
    }
  }

  void visit(ast::un_operator *stm) override {
    stm->arg()->accept(this);

    switch(stm->type()) {
      case ast::UnOp::PLUS :
        set_value(+get_value());
        break;
      case ast::UnOp::MINUS :
        set_value(-get_value());
        break;
      case ast::UnOp::NEGATE :
        set_value(!get_value());
        break;
      default: throw std::logic_error{"unrecognized logic type"};
    }
  }

  void visit(ast::logic_expression *stm) override {
    stm->left()->accept(this);
    auto lhs = get_value();
    stm->right()->accept(this);
    auto rhs = get_value();

    switch(stm->type()) {
      case ast::LogicOp::LESS :
        set_value(lhs <  rhs);
        break;
      case ast::LogicOp::LESS_EQ :
        set_value(lhs <= rhs);
        break;
      case ast::LogicOp::LOGIC_AND :
        set_value(lhs && rhs);
        break;
      case ast::LogicOp::LOGIC_OR :
        set_value(lhs || rhs);
        break;
      case ast::LogicOp::GREATER:
        set_value(lhs > rhs);
        break;
      case ast::LogicOp::GREATER_EQ :
        set_value(lhs >= rhs);
        break;
      case ast::LogicOp::EQ :
        set_value(lhs == rhs);
        break;
      case ast::LogicOp::NEQ :
        set_value(lhs != rhs);
        break;
      default: throw std::logic_error{"unrecognized logic type"};
    }
  }

  void visit(ast::integer_literal *stm) override {
    set_value(stm->get_value());
  }

  void visit(ast::integer_variable *stm) override {
    set_value(stm->get_value());
  }

  void visit(ast::assignment<int> *stm) override {
    std::cout << "IN ASSIGNMENT" << std::endl;
    stm->ident_exp()->accept(this);
    stm->redefine(get_value());
  }

  void visit(ast::read_expression*) override {
    int tmp {0};
    input_stream_ >> tmp;
    set_value(tmp);
  }

  void visit(ast::if_operator *stm) override {
    stm->condition()->accept(this);
    if(get_value()) {
      stm->body()->accept(this);
    } else if (stm->else_block()) {
      stm->else_block()->accept(this);
    }
  }

  void visit(ast::while_operator *stm) override {
    stm->condition()->accept(this);

    while(get_value()) {
      stm->body()->accept(this);
      stm->condition()->accept(this);
    }
  }

  void visit(ast::print_function *stm) override {
    stm->get()->accept(this);
    output_stream_ << get_value() << std::endl;
  }

  void visit(ast::array_elem *stm) override {
    std::cout << "name = " << stm->name() << std::endl;
    std::cout << "indexes:\n";
    auto inxs = stm->indexes();
    for (auto i : inxs) {
      i->accept(this);
      std::cout << get_value() << std::endl;
    }
  }

  void visit(ast::array *) override {

  }

  void run_program(ast::statement_block *root) {
    visit(root);
  }

 private:
  std::istream &input_stream_;
  std::ostream &output_stream_;
};

} // <--- namespace frontend
