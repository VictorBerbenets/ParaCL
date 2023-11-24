%language "c++"

%skeleton "lalr1.cc"
%require "3.5"

%defines
%define api.token.raw
%define api.parser.class { parser }
%define api.value.type variant
%define api.token.constructor
%define api.namespace { yy }
%define parse.lac full

%locations

%code requires {

#include <iostream>
#include <string>
#include <stdexcept>
#include <utility>

#include "syntax.hpp"

namespace yy {
  class scanner;
  class Driver;
}

using namespace yy;

}

%code top {

#include <iostream>
#include <string>

#include "driver.hpp"
#include "paracl_grammar.tab.hh"
#include "scanner.hpp"

static yy::parser::symbol_type yylex(yy::scanner &p_scanner, yy::Driver &p_driver) {
  return p_scanner.get_next_token();
}

}

%param {yy::scanner& Scanner}
%param {yy::Driver& Driver}

%define parse.trace
%define parse.error verbose
%define api.token.prefix {TOKEN_}

%token
  ASSIGN   "="
  MUL      "*"
  DIV      "/"
  PLUS     "+"
  MINUS    "-"
  EOF 0 "end of file"
  SCOLON  ";"
;

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

// logical tokens
%token
  EQ  "=="
  NEQ "!="
  LESS "<"
  LESS_EQ "<="
  GREATER ">"
  GREATER_EQ ">="

  LOGIC_AND "&&"
  LOGIC_OR "||"
;

%token <int> NUMBER
%token <std::string> VAR

%nonassoc LESS LESS_EQ GREATER GREATER_EQ
%right ASSIGN
%left PLUS MINUS
%left MUL DIV
%left EQ NEQ
%nonassoc UMINUS
%nonassoc UPLUS

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

exp:    logical_expression           {}
      | bin_operation                {}
      | unary_operation              {}
      | OP_BRACK exp CL_BRACK        { std::cout << __LINE__ << std::endl;   }
      | VAR ASSIGN exp               { std::cout << $1 << " = " << "EXP" << std::endl; }
      | NUMBER                       { std::cout << "NUMBER = " << $1 << std::endl; }
      | VAR                          { std::cout << "VAR = " << $1 << std::endl;    }
;

unary_operation:  MINUS exp %prec UMINUS       { std::cout << "UMINUS" << std::endl; }
                 | PLUS  exp %prec UPLUS       { std::cout << "UPLUS" <<std::endl;   }
;

bin_operation:   exp PLUS  exp                { std::cout << "PLUS" << std::endl;     }
               | exp MINUS exp                { std::cout << "MINUS" << std::endl;    }
               | exp MUL exp                  { std::cout << "MUL" << std::endl;      }
               | exp DIV exp                  { std::cout << "DIV" << std::endl;      }
;

logical_expression:   exp LESS exp        { std::cout << "LESS"    << std::endl;  }
                    | exp LESS_EQ exp     { std::cout << "LESS EQ" << std::endl;  }
                    | exp GREATER exp     { std::cout << "GREATER " << std::endl; }
                    | exp GREATER_EQ exp  { std::cout << "GREATER EQ"    << std::endl; }
                    | exp EQ exp          { std::cout << "EQ"  << std::endl; }
                    | exp NEQ exp         { std::cout << "NEQ" << std::endl; }
;

function:  scan_func
         | print_func
;

scan_func: VAR ASSIGN SCAN SCOLON   { std::cout << "SCAN FUNC" << std::endl; }
;

print_func:  PRINT NUMBER SCOLON       { std::cout << "PRINT " << $2 << std::endl; }
           | PRINT VAR SCOLON         { std::cout << "PRINT " << $2 << std::endl; }
;

operator: IF OP_BRACK exp CL_BRACK OP_BRACE statement_block CL_BRACE          { std::cout << "IF\n"; }
          | WHILE OP_BRACK exp CL_BRACK OP_BRACE statement_block CL_BRACE     { std::cout << "WHILE" << std::endl;}
;

%%


void yy::parser::error(const location_type& l, const std::string &msg) {
  std::cout << "error pos: " << l << std::endl;
  throw std::runtime_error{msg};
}

