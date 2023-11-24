#pragma once

#include "statement.hpp"

namespace frontend {

namespace ast {

// expr nodes
class expression: public statement {
  protected:
    using pointer_type = std::unique_ptr<expression>;

    virtual pointer_type eval() const = 0;
};

class ctrl_statement: public statement {
  protected:
    using expr_ptr = std::unique_ptr<expression>;
    using stmt_ptr = std::unique_ptr<statement>;

    expr_ptr condition_;
    stmt_ptr body_;
};

class bin_operator: public expression{
  protected:
    bin_operator(BinOp type, pointer_type left_, pointer_type right_);

    pointer_type eval() const override;

    BinOp type_;
    pointer_type left_, right_;
};

class un_operator: public expression {
  protected:
    un_operator(UnOp type, pointer_type child);

    pointer_type eval() const override;

    UnOp type_;
    pointer_type child_;
};

class add_expression: public bin_operator {

};

class sub_expression: public bin_operator {

};

class mul_expression: public bin_operator {

};

class div_expression: public bin_operator {

};


} // <--- namespace ast

} // <--- namespace frontend
