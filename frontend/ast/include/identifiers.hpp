#pragma once

namespace frontend {

namespace ast {

enum class CtrlStatement: char {
  IF,
  WHILE
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

} // <--- namespace ast

} // <--- namespace frontend

