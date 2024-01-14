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

enum class CalcOp: char {
    PERCENT,
    ADD,
    SUB,
    MUL,
    DIV
};

enum class UnOp: char {
    PLUS,
    MINUS,
    NEGATE
};

} // <--- namespace ast

} // <--- namespace frontend

