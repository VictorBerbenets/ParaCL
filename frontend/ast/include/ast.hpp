#pragma once

#include <memory>
#include <vector>

#include "statement.hpp"

namespace frontend {

namespace ast {

class ast final {
  public:

  private:
    std::unique_ptr<statement> root_;
};

class qualifier: public statement {

};

} // <--- namespace ast

} // <--- namespace frontend

