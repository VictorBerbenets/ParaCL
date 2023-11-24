#pragma once

#include <memory>
#include <list>

namespace frontend {

namespace ast {

class statement {
  public:
    virtual ~statement() = default;

  private:
    statement* parent_;
};

class statement_block final {
  private:
    std::list<std::unique_ptr<statement>> statements_;
};

} // <--- namespace ast

} // <--- namespace frontend
