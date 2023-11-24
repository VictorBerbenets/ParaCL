%language "c++"

%skeleton "lalr1.cc"
%require "3.5"

%defines
%define api.token.raw
%define api.parser.class { parser }
%define api.value.type variant
%define api.token.constructor
%define api.namespace { my_yy }
%define parse.lac full

%locations

%code requires {

#include <iostream>
#include <string>
#include <stdexcept>
#include <utility>

#include "syntax.hpp"

namespace my_yy {
  class scanner;
  class Driver;
}

using namespace my_yy;

}

%code top {

#include <iostream>
#include <string>

#include "driver.hpp"
#include "paracl_grammar.tab.hh"
#include "scanner.hpp"

static my_yy::parser::symbol_type yylex(my_yy::scanner &p_scanner, my_yy::Driver &p_driver) {
  return p_scanner.get_next_token();
}

}

%param {my_yy::scanner& Scanner}
%param {my_yy::Driver& Driver}

%define parse.trace
%define parse.error verbose
%define api.token.prefix {TOKEN_}


%token  ASSIGN   "="
%token  MUL      "*"
%token  DIV      "/"
%token  PLUS     "+"
%token  MINUS    "-"

%token EOF 0 "end of file"
%token SCOLON  ";"

// statement tokens
%token  IF         "if"
%token  WHILE      "while"
%token  PRINT      "print"
%token  SCAN       "?"
%token  OP_BRACK   "("
%token  CL_BRACK   ")"
%token  OP_BRACE   "{"
%token  CL_BRACE   "}"


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

statement_block: %empty {}
                | statement_block statement { std::cout << "statement\n" << std::endl; }

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
      | VAR ASSIGN exp SCOLON        { std::cout << $1 << " = " << "EXP" << std::endl; }
      | NUMBER                       { std::cout << "NUMBER = " << $1 << std::endl; }
      | VAR                          { std::cout << "VAR = " << $1 << std::endl;    }
;
%%


void my_yy::parser::error(const location_type& l, const std::string &msg) {
  std::cout << "ERROR\n";
  std::cout << l << std::endl;
  //throw std::runtime_error{msg};
}
