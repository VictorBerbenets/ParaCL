#include "visitor.hpp"
#include "ast_includes.hpp"

namespace frontend {

int visitor::accept(ast::statement* node) {
    node->accept(this);
    return curr_value_;
}

} // <--- namespace frontend
