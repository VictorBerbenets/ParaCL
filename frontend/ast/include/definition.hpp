#pragma once

#include <string>

#include "statement.hpp"
#include "expression.hpp"

namespace frontend {

namespace ast {

class definition: public statement {
protected:
    definition(const std::string& name): name_ {name} {}
    definition(std::string&& name): name_ {std::move(name)} {}

    std::string name_;
};

class var_definition: public definition {
 public:
    var_definition(const std::string& name, expression* expr);
    var_definition(&& name, expression* expr);
    ~var_definition() override = default;

 private:
  expression* identifier_;
};

} // <--- namespace ast

} // <--- namespace frontend
