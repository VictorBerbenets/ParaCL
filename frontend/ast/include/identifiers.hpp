#pragma once

namespace frontend {

namespace ast {

enum class LogicOp: char {
  LESS,
  LESS_EQ,
  GREATER,
  GREATER_EQ,
  EQ,
  NEQ,
  LOGIC_AND,
  LOGIC_OR
};

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

