%option c++
%option nodefault yylineno

%{

#include "paracl_grammar.tab.hh"
#include "driver.hpp"

%}

WS      [ \t\n\v]+
DIGIT   [0-9]
DIGIT1  [1-9]

%{

  #define YY_USER_ACTION  loc.columns (yyleng);
%}


%%

%{
  yy::location& loc = driver->location();
  // Code run each time yylex is called.
  loc.step ();
%}

{WS}
"+"              { return yy::parser::make_PLUS(loc);   }
"-"              { return yy::parser::make_MINUS(loc);  }
"*"              { return yy::parser::make_MUL(loc);    }
"/"              { return yy::parser::make_DIV(loc);    }
"="              { return yy::parser::make_ASSIGN(loc); }
";"              { return yy::parser::make_SCOLON(loc); }

">"  |
"<"  |
">=" |
"<=" |
"==" |
"!="              { return yy::parser::make_CMP(1, loc); }


"if"              { return yy::parser::make_IF(loc);       }
"while"           { return yy::parser::make_WHILE(loc);    }
"print"           { return yy::parser::make_PRINT(loc);    }
"?"               { return yy::parser::make_SCAN(loc);     }
"("               { return yy::parser::make_OP_BRACK(loc); }
")"               { return yy::parser::make_CL_BRACK(loc); }
"{"               { return yy::parser::make_OP_BRACE(loc); }
"}"               { return yy::parser::make_CL_BRACE(loc); }

-?{DIGIT1}{DIGIT}* { return yy::parser::make_NUMBER(std::atoi(yytext), loc); }
[a-zA-Z]+          { return yy::parser::make_VAR(yytext, loc);               }
.			             { return yy::parser::make_ERR(loc);                       }

%%