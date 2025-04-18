%{
#include <llvm/ADT/StringRef.h>

#include <iostream>
#include <numeric>

#include "scanner.hpp"

#define yyterminate()

using namespace yy;
%}

%option noyywrap nounput noinput nodefault
%option c++
%option yyclass="scanner"
%option yylineno

WS      [ \t\v]+
DIGIT   [0-9]
DIGIT1  [1-9]
VAR     [a-zA-Z_][a-zA-Z_0-9_]*

%%
([/][/].*)        { update_location(); }
([\n]+)           { update_location(); }
{WS}              { update_location(); }
"+"               { return parser::make_PLUS(update_location());    }
"-"               { return parser::make_MINUS(update_location());   }
"*"               { return parser::make_MUL(update_location());     }
"/"               { return parser::make_DIV(update_location());     }
"%"               { return parser::make_PERCENT(update_location()); }
"="               { return parser::make_ASSIGN(update_location());  }
";"               { return parser::make_SCOLON(update_location());  }
","               { return parser::make_COMMA(update_location());   }

">"               { return parser::make_GREATER(update_location());    }
"<"               { return parser::make_LESS(update_location());       }
">="              { return parser::make_GREATER_EQ(update_location()); }
"<="              { return parser::make_LESS_EQ(update_location());    }
"=="              { return parser::make_EQ(update_location());         }
"!="              { return parser::make_NEQ(update_location());        }
"&&"              { return parser::make_LOGIC_AND(update_location());  }
"||"              { return parser::make_LOGIC_OR(update_location());   }
"!"               { return parser::make_NEGATE(update_location());     }

"if"              { return parser::make_IF(update_location());       }
"else"            { return parser::make_ELSE(update_location());     }
"while"           { return parser::make_WHILE(update_location());    }
"print"           { return parser::make_PRINT(update_location());    }
"array"           { return parser::make_ARRAY(update_location());    }
"repeat"          { return parser::make_REPEAT(update_location());   }
"undef"           { return parser::make_UNDEF(update_location());    }

"?"               { return parser::make_SCAN(update_location());       }
"("               { return parser::make_OP_BRACK(update_location());   }
")"               { return parser::make_CL_BRACK(update_location());   }
"{"               { return parser::make_OP_BRACE(update_location());   }
"}"               { return parser::make_CL_BRACE(update_location());   }
"["               { return parser::make_OP_SQBRACK(update_location()); }
"]"               { return parser::make_CL_SQBRACK(update_location()); }

{DIGIT1}?{DIGIT}* { return parser::make_NUMBER(std::atoi(yytext), update_location()); }
{VAR}               { return parser::make_VAR(llvm::StringRef(yytext), update_location()); }
.			        { throw parser::syntax_error{update_location(), "Unknown symbol"};  }
<<EOF>>             { return parser::make_EOF(update_location());}

%%
