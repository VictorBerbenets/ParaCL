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
  ASSIGN   '='
  MUL      '*'
  DIV      '/'
  PLUS     '+'
  MINUS    '-'
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

%nonassoc <int> CMP
%right ASSIGN
%left PLUS MINUS
%left MUL DIV
%nonassoc UMINUS

%start program

%%

program: statement_block
;

statement_block: %empty
                 | statement statement_block
;

statement: exp SCOLON
          | operator
;
// shift-reduce conflicts
exp:    exp CMP exp
      | exp PLUS exp
      | exp MINUS exp
      | exp MUL exp
      | exp DIV exp
      | exp ASSIGN exp
      | '(' exp ')'
      | MINUS exp %prec UMINUS
      | NUMBER
      | VAR
      | VAR ASSIGN exp
      | function
;

operator: IF '(' exp ')' '{' statement '}'
          | WHILE '(' exp ')' '{' statement '}'
;

function: VAR ASSIGN SCAN
          | PRINT VAR


%%

namespace yy {

parser::token_type yylex(parser::semantic_type* yylval,
                         NumDriver* driver)
{
  return driver->yylex(yylval);
}

void parser::error(const std::string&){}
}
