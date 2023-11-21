%option c++
%option nodefault yylineno

%{

#include "paracl_grammar.tab.hh"

%}

WS      [ \t\n\v]+
DIGIT   [0-9]
DIGIT1  [1-9]
/* VAR     [a-zA-Z][a-zA-Z0-9]+*/

%%

{WS}
"+"              { return yy::parser::token_type::PLUS;   }
"-"              { return yy::parser::token_type::MINUS;  }
"*"              { return yy::parser::token_type::MUL;    }
"/"              { return yy::parser::token_type::DIV;    }
"="              { return yy::parser::token_type::ASSIGN; }
";"              { return yy::parser::token_type::SCOLON; }

">"  |
"<"  |
">=" |
"<=" |
"==" |
"!="              { return yy::parser::token_type::CMP; }


"if"              { std::cout << yytext << std::endl; return yy::parser::token_type::IF;       }
"while"           { return yy::parser::token_type::WHILE;    }
"print"           { return yy::parser::token_type::PRINT;    }
"?"               { return yy::parser::token_type::SCAN;     }
"("               { return yy::parser::token_type::OP_BRACK; }
")"               { return yy::parser::token_type::CL_BRACK; }
"{"               { return yy::parser::token_type::OP_BRACE; }
"}"               { return yy::parser::token_type::CL_BRACE; }

-?{DIGIT1}{DIGIT}* { return yy::parser::token_type::NUMBER;  }
[a-zA-Z]+          {    //return yy::parser::make_IDENTIFIER(yytext, loc);
                    /*return yy::parser::token_type::VAR; */    
                   }
.			       { return yy::parser::token_type::ERR;     }

%%

