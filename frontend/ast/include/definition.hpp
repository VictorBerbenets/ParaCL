#pragma once

#include <string>
#include <utility>

#include "statement.hpp"
#include "expression.hpp"
#include "visitor.hpp"

namespace frontend {

namespace ast {

class definition: public statement {
public:
    definition(const std::string &name): name_ {name} {}
    definition(std::string &&name): name_ {std::move(name)} {}
    ~definition() override = default;

    // void accept() override;

    std::string name_;
};

class var_definition: public definition {
 public:
    var_definition(const std::string &name, expression *expr);
    var_definition(std::string &&name, expression *expr);
    ~var_definition() override = default;

    void accept(base_visitor *base_visitor) override;

//  private:
  expression* identifier_;
};

} // <--- namespace ast

} // <--- namespace frontend
