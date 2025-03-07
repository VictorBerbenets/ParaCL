#pragma once

namespace paracl {

namespace ast {

enum class LogicOp : char {
  LESS,
  LESS_EQ,
  GREATER,
  GREATER_EQ,
  EQ,
  NEQ,
  AND,
  OR
};

enum class CalcOp : char { PERCENT, ADD, SUB, MUL, DIV };

enum class UnOp : char { PLUS, MINUS, NEGATE };

} // namespace ast

} // namespace paracl
