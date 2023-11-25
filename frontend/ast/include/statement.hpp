#pragma once

#include <memory>
#include <list>

// #include "visitor"

namespace frontend {

namespace ast {

class statement {
  public:
    //virtual void accept(visitor* visitor) = 0;
    virtual ~statement() = default;

  private:
    statement* parent_;
};

class statement_block final {
  private:
    std::list<statement*> statements_;
};

} // <--- namespace ast

} // <--- namespace frontend
