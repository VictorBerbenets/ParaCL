#pragma once

#include "statement.hpp"

namespace frontend {

namespace ast {


enum class BinOp: char {
    ADD,
    SUB,
    MUL,
    DIV
};

enum class UnOp: char {
    PLUS,
    MINUS
};


// expr nodes
class expression: public statement {
  public:
    ~expression() override = default;

  protected:
    using pointer_type = std::unique_ptr<expression>;

    virtual pointer_type eval() const = 0;

};

class number: public expression {
  public:
    number(int num);

    pointer_type eval() const;

    ~number() override = default;
  private:
    int value_;
};

class variable: public expression {
  public:
    variable(const std::string& str);
    variable(std::string&& str);

    pointer_type eval() const;
  private:
  std::string name_;
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


} // <--- namespace ast

} // <--- namespace frontend
