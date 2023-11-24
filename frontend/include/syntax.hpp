#pragma once

#include <memory>
#include <vector>

namespace frontend {
/*
ast tree
eval

*/

namespace ast {

enum class NodeT: char {

};

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

class statement;

class ast {
  public:

  private:
    std::unique_ptr<statement> root_;
};

class statement {
  public:
    virtual ~statement() {}

  private:
    statement* parent_;
};

class qualifier: public statement {

};
// expr nodes
class expression: public statement {
  protected:
    using pointer_type = std::unique_ptr<expression>;

    virtual pointer_type eval() const = 0;
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


/////////////////////////////////////////

// ctrl stmt nodes
class ctrl_statement: public statement {
  protected:
    using expr_ptr = std::unique_ptr<expression>;
    using stmt_ptr = std::unique_ptr<statement>;

    expr_ptr condition_;
    stmt_ptr body_;
};

class while_operator: public ctrl_statement {

};

class if_operator: public ctrl_statement {

};

/////////////////////////////////////////

// func nodes
class function: public statement {

};

class print_function: public function {

};

class scan_function: public function {

};
/////////////////////////////////////////


} // <--- namespace ast

} // <--- namespace frontend

