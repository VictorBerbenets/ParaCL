%language "c++"

%skeleton "lalr1.cc"
%defines
%define api.value.type variant
%param {yy::NumDriver* driver}

%code requires
{
#include <iostream>
#include <string>
#include <utility>

// forward decl of argument to parser
namespace yy { class NumDriver; }
}

%code
{

#include "driver.hpp"

namespace yy {

parser::token_type yylex(parser::semantic_type* yylval,
                         NumDriver* driver);
}

}

// ariphmetic tokens
%token
  ASSIGN   "="
  MUL      "*"
  DIV      "/"
  ADD      "+"
  SUB      "-"
  LT       "<"
  GT       ">"
  LE       "<="
  GE       ">="
  EQ       "=="
  NEQ      "!="
  ERR
;

%token SCOLON  ";"

// statement tokens
%token
  IF         "if"
  WHILE      "while"
  PRINT      "print"
  SCAN       "?"
  OP_BRACK   "("
  CL_BRACK   ")"
  OP_BRACE   "{"
  CL_BRACE   "}"
;


%token <int> NUMBER
%token <std::string> VAR
%nterm <int> equals
%nterm <int> expr

%left '+' '-' '*' '/'

%start program

%%

program: eqlist
;

eqlist: equals SCOLON eqlist
      | %empty
;

equals: expr ASSIGN expr       { 
                                $$ = ($1 == $3); 
                                std::cout << "Checking: " << $1 << " vs " << $3 
                                          << "; Result: " << $$
                                          << std::endl; 
                              }
;

expr: expr ADD NUMBER        { $$ = $1 + $3; }
    | expr SUB NUMBER       { $$ = $1 - $3; }
    | NUMBER                  { $$ = $1; }
;

%%

namespace yy {

parser::token_type yylex(parser::semantic_type* yylval,                         
                         NumDriver* driver)
{
  return driver->yylex(yylval);
}

void parser::error(const std::string&){}
}
