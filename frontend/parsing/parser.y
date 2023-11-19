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

#include "syntax.hpp"

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

statement_block: statement statement_block { std::cout << __LINE__ << std::endl; }
                | %empty
;

statement: exp SCOLON   {std::cout << __LINE__ << std::endl;}
          | operator    {std::cout << __LINE__ << std::endl;}
          | function    {std::cout << __LINE__ << std::endl;}
;

exp:    exp CMP exp                  {std::cout << __LINE__ << std::endl;}
      | exp PLUS exp                 {std::cout << __LINE__ << std::endl;}
      | exp MINUS exp                {std::cout << __LINE__ << std::endl;}
      | exp MUL exp                  {std::cout << __LINE__ << std::endl;}
      | exp DIV exp                  {std::cout << __LINE__ << std::endl;}
      | '(' exp ')'                  {std::cout << __LINE__ << std::endl;}
      | MINUS exp %prec UMINUS       {std::cout << __LINE__ << std::endl;}
      | NUMBER                       {std::cout << __LINE__ << std::endl;}
      | VAR                          {std::cout << __LINE__ << std::endl;}
      | VAR ASSIGN exp               {std::cout << __LINE__ << std::endl;}
;

operator: IF '(' exp ')' '{' statement '}'        { std::cout << "IF\n"; }
          | WHILE '(' exp ')' '{' statement '}'   {std::cout << __LINE__ << std::endl;}
;

function: VAR ASSIGN SCAN       { std::cout << __LINE__ << std::endl;}
          | PRINT VAR           { std::cout << "HI\n" << std::endl; std::cout << $2 << std::endl; }
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
