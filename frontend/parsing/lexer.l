%{
#include <iostream>
#include <numeric>

#include "scanner.hpp"

#define yyterminate()

using namespace yy;
%}

%option noyywrap nounput noinput nodefault
%option c++
%option yyclass="scanner"

WS      [ \t\n\v]+
DIGIT   [0-9]
DIGIT1  [1-9]

%%

{WS}
"+"              { return yy::parser::make_PLUS();   }
"-"              { return yy::parser::make_MINUS();  }
"*"              { return yy::parser::make_MUL();    }
"/"              { return yy::parser::make_DIV();    }
"="              { return yy::parser::make_ASSIGN(); }
";"              { return yy::parser::make_SCOLON(); }

">"  |
"<"  |
">=" |
"<=" |
"==" |
"!="              { return yy::parser::make_CMP(); }


"if"              { return yy::parser::make_IF();       }
"while"           { return yy::parser::make_WHILE();    }
"print"           { return yy::parser::make_PRINT();    }
"?"               { return yy::parser::make_SCAN();     }
"("               { return yy::parser::make_OP_BRACK(); }
")"               { return yy::parser::make_CL_BRACK(); }
"{"               { return yy::parser::make_OP_BRACE(); }
"}"               { return yy::parser::make_CL_BRACE(); }

-?{DIGIT1}{DIGIT}* { return yy::parser::make_NUMBER(std::atoi(yytext)); }
[a-zA-Z]+          { return yy::parser::make_VAR(yytext);               }
.			             { return yy::parser::make_ERR();                     }

%%