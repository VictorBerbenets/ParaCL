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
    definition(statement_block *curr_block, const std::string &name)
        : statement(curr_block), name_ {name} {}
    definition(statement_block *curr_block, std::string &&name)
        : statement(curr_block), name_ {std::move(name)} {}
    ~definition() override = default;

    // void accept() override;

    std::string name_;
};

class assignment: public definition {
 public:
    assignment(statement_block *curr_block, const std::string &name, expression *expr);
    assignment(statement_block *curr_block, std::string &&name, expression *expr);
    ~assignment() override = default;

    void accept(base_visitor *base_visitor) override;

//  private:
  expression* identifier_;
};

} // <--- namespace ast

} // <--- namespace frontend
