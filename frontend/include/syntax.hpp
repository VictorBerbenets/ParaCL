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
    
}

enum class BinOp: char {
    ADD,
    SUB,
    MUL,
    DIV
}


class ast {
public:
    
private:
    std::unique_ptr<stmt_node> root_;
};

class stmt_node {

    virtual ~stmt_node() {}

private:
    stmt_node* parent_;
    NodeT type_;

};

class decl_node: public stmt_node {

};
// expr nodes
class expr_node: public stmt_node {
    using pointer_type = std::unique_ptr<expr_node>;
private:
    std::vector<pointer_type> expr_childs_;
};

class bin_op_node: public expr_node {

};

class un_op_node: public expr_node {

};
/////////////////////////////////////////

// ctrl stmt nodes
class ctrl_stmt_node: public stmt_node {

};

class while_node: public ctrl_stmt_node {

};

class if_node: public ctrl_stmt_node {

};

/////////////////////////////////////////

// func nodes
class func_node: public stmt_node {

};

class print_node: public func_node {

};

class scan_node: public func_node {

};
/////////////////////////////////////////


} // <--- namespace ast

} // <--- namespace frontend

