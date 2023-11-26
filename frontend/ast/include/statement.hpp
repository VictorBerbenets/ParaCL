#pragma once

#include <memory>
#include <list>

// #include "visitor"

namespace frontend {

namespace ast {

class ast_node {
 public:
  virtual ~ast_node() = default;
};

class statement: public ast_node {
 public:
  //virtual void accept(visitor* visitor) = 0;
 // virtual ~statement() = default;

 private:
  statement* parent_;
};



class statement_block: public statement {
 public:
  void add(statement* stm) {
    statements_.push_back(stm);
  }
 private:
  std::list<statement*> statements_;
};

} // <--- namespace ast

} // <--- namespace frontend
