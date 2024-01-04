#pragma once

#include <vector>

#include "expression.hpp"
#include "visitor.hpp"
#include "location.hh"

namespace frontend {

namespace ast {

class ctrl_statement: public statement {
 public:
   ctrl_statement(expression *cond, statement_block *body, yy::location loc)
      : statement {loc},
        condition_ {cond},
        body_ {body} {}

  ~ctrl_statement() override = default;

  expression *condition() const noexcept {
    return condition_;
  }

  statement_block *body() const noexcept {
    return body_;
  }

  void accept_body(base_visitor *b_visitor) {
    body_->accept(b_visitor);
  }

  void accept_condition(base_visitor *b_visitor) {
    condition_->accept(b_visitor);
  }

 protected:
  expression *condition_;
  statement_block *body_;
};

class while_operator: public ctrl_statement {
 public:
  using ctrl_statement::ctrl_statement;

  void accept(base_visitor *b_visitor) override {
    b_visitor->visit(this);
  }
};

class if_operator: public ctrl_statement {
  using else_if_vector = std::vector<ctrl_statement>
 public:
  using ctrl_statement::ctrl_statement;
  
  if_operator(expression *cond, statement_block *body, else_if_vector &&vec, 
              statement_block *else_node, yy::location loc)
      : ctrl_statement {cond, body, loc},
        else_if_nodes_ {std::move(vec)},
        else_node_ {else_node} {}

  void accept(base_visitor *b_visitor) override {
    b_visitor->visit(this);
  }
  
  statement_block *else_node() noexcept { return else_node_; }

  auto begin()  noexcept { return else_if_nodes_.begin();  }
  auto end()    noexcept { return else_if_nodes_.end();    }
  auto cbegin() noexcept { return else_if_nodes_.cbegin(); }
  auto cend()   noexcept { return else_if_nodes_.cend();   }
 private:
  else_if_vector else_if_nodes_;
  statement_block *else_node_;
};


} // <--- namespace ast

} // <--- namespace frontend
