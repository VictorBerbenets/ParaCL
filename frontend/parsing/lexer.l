%{
#include <iostream>
#include <numeric>

#include "scanner.hpp"

#define yyterminate()

using namespace my_yy;
%}

%option noyywrap nounput noinput nodefault
%option c++
%option yyclass="scanner"

WS      [ \t\n\v]+
DIGIT   [0-9]
DIGIT1  [1-9]

%%

{WS}
"+"              { return parser::make_PLUS(update_loc());   }
"-"              { return parser::make_MINUS(update_loc());  }
"*"              { return parser::make_MUL(update_loc());    }
"/"              { return parser::make_DIV(update_loc());    }
"="              { return parser::make_ASSIGN(update_loc()); }
";"              { return parser::make_SCOLON(update_loc()); }

">"  |
"<"  |
">=" |
"<=" |
"==" |
"!="              { return parser::make_CMP(1, update_loc()); }


"if"              { return parser::make_IF(update_loc());       }
"while"           { return parser::make_WHILE(update_loc());    }
"print"           { return parser::make_PRINT(update_loc());    }
"?"               { return parser::make_SCAN(update_loc());     }
"("               { return parser::make_OP_BRACK(update_loc()); }
")"               { return parser::make_CL_BRACK(update_loc()); }
"{"               { return parser::make_OP_BRACE(update_loc()); }
"}"               { return parser::make_CL_BRACE(update_loc()); }

-?{DIGIT1}{DIGIT}* { return parser::make_NUMBER(std::atoi(yytext), update_loc()); }
[a-zA-Z]+          { return parser::make_VAR(yytext, update_loc());               }
.			             { throw parser::syntax_error{update_loc(), "Unknown symbol"}; }
<<EOF>>            { return parser::make_EOF(update_loc());}

%%