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
([/][/].*)       { update_location(); }
{WS}             { update_location(); }
"+"              { return parser::make_PLUS(update_location());   }
"-"              { return parser::make_MINUS(update_location());  }
"*"              { return parser::make_MUL(update_location());    }
"/"              { return parser::make_DIV(update_location());    }
"="              { return parser::make_ASSIGN(update_location()); }
";"              { return parser::make_SCOLON(update_location()); }

">"  |
"<"  |
">=" |
"<=" |
"==" |
"!="              { return parser::make_CMP(1, update_location()); }


"if"              { return parser::make_IF(update_location());       }
"while"           { return parser::make_WHILE(update_location());    }
"print"           { return parser::make_PRINT(update_location());    }
"?"               { return parser::make_SCAN(update_location());     }
"("               { return parser::make_OP_BRACK(update_location()); }
")"               { return parser::make_CL_BRACK(update_location()); }
"{"               { return parser::make_OP_BRACE(update_location()); }
"}"               { return parser::make_CL_BRACE(update_location()); }

-?{DIGIT1}{DIGIT}* { return parser::make_NUMBER(std::atoi(yytext), update_location()); }
[a-zA-Z]+          { return parser::make_VAR(yytext, update_location());               }
.			       { throw parser::syntax_error{update_location(), "Unknown symbol"}; }
<<EOF>>            { return parser::make_EOF(update_location());}

%%