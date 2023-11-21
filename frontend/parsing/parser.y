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

statement_block: statement statement_block { std::cout << "statement" << std::endl; }
                | %empty
;

statement: exp SCOLON   {std::cout << "EXPRESSION" << std::endl;}
          | operator    {std::cout << "OPERATOR" << std::endl;}
          | function    {std::cout << "FUNCTION" << std::endl;}
;

operator: IF OP_BRACK exp CL_BRACK OP_BRACE statement_block CL_BRACE          { std::cout << "IF\n"; }
          | WHILE OP_BRACK exp CL_BRACK OP_BRACE statement_block CL_BRACE     { std::cout << "WHILE" << std::endl;}
;

function: scan_func
         | print_func
;

scan_func: VAR ASSIGN SCAN SCOLON   { std::cout << "SCAN FUNC" << std::endl; }
;

print_func: PRINT NUMBER SCOLON       { std::cout << "PRINT " << $2 << std::endl; }
           | PRINT VAR SCOLON         { std::cout << "PRINT " << $2 << std::endl; }
;

exp:    exp CMP exp                  { std::cout << "EXP COMP" << std::endl; }
      | exp PLUS exp                 { std::cout << "PLUS" << std::endl;     }
      | exp MINUS exp                { std::cout << "MINUS" << std::endl;    }
      | exp MUL exp                  { std::cout << "MUL" << std::endl;      }
      | exp DIV exp                  { std::cout << "DIV" << std::endl;      }
      | OP_BRACK exp CL_BRACK        { std::cout << __LINE__ << std::endl;   }
      | MINUS exp %prec UMINUS       { std::cout << __LINE__ << std::endl;   }
      | NUMBER                       { std::cout << "NUMBER = " << $1 << std::endl; }
      | VAR                          { std::cout << "VAR = " << $1 << std::endl;    }
      | VAR ASSIGN exp SCOLON        { std::cout << $1 << " = " << "EXP" << std::endl; }
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
