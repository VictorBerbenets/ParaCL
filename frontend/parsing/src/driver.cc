#include <iostream>

#include <FlexLexer.h>

#include "driver.hpp"

// here we can return non-zero if lexing is not done inspite of EOF detected
int yyFlexLexer::yywrap() { return 1; }

int main(int argc, char** argv) {
  FlexLexer *lexer = new yyFlexLexer;
  yy::Driver driver(lexer);
  driver.parse(argv[1]);
  delete lexer;
}
