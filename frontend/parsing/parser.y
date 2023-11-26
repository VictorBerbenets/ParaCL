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

// #include "ast.hpp"

#include "ast_includes.hpp"

namespace yy {
  class scanner;
  class Driver;
}

using namespace yy;
using namespace frontend::ast;

}

%code top {

#include <iostream>
#include <string>

#include "driver.hpp"
#include "scanner.hpp"
#include "paracl_grammar.tab.hh"

static yy::parser::symbol_type yylex(yy::scanner &p_scanner, yy::Driver &p_driver) {
  return p_scanner.get_next_token();
}

}

%param {yy::scanner& scanner}
%param {yy::Driver& driver}

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

// terminal tokens
%token <int> NUMBER
%token <std::string> VAR

%nterm <statement*>          statement
%nterm <statement_block*>    statement_block
%nterm <expression*>         exp
%nterm <logic_expression*> logical_expression
%nterm <bin_operator*>       bin_operation
%nterm <un_operator*>        unary_operation
%nterm <function*>           function
%nterm <ctrl_statement*>     ctrl_statement

%nonassoc LESS LESS_EQ GREATER GREATER_EQ
%right ASSIGN
%left PLUS MINUS
%left MUL DIV
%left EQ NEQ LOGIC_AND LOGIC_OR
%nonassoc UMINUS
%nonassoc UPLUS

// %nterm <exp> int

%start program

%%

program: statement_block {}
;

statement_block:  %empty {}
                | statement_block statement { std::cout << "statement\n" << std::endl; }
;

statement:  exp SCOLON        {std::cout << "EXPRESSION" << std::endl;}
          | ctrl_statement    {std::cout << "OPERATOR" << std::endl;}
          | function          {std::cout << "FUNCTION" << std::endl;}
;

exp:    logical_expression           {}
      | bin_operation                {}
      | unary_operation              {}
      | OP_BRACK exp CL_BRACK        { std::cout << __LINE__ << std::endl;   }
      | VAR ASSIGN exp               { std::cout << $1 << " = " << "EXP" << std::endl; }
      | NUMBER                       { std::cout << "NUMBER = " << $1 << std::endl; }
      | VAR                          { std::cout << "VAR = " << $1 << std::endl;    }
;

unary_operation:   MINUS exp %prec UMINUS      { std::cout << "UMINUS" << std::endl; }
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
                    | exp LOGIC_AND exp   { std::cout << "LOGIC AND" << std::endl;}
                    | exp LOGIC_OR exp    { std::cout << "LOGIC OR" << std::endl;}
;

function:  VAR ASSIGN SCAN SCOLON    { std::cout << "SCAN FUNC" << std::endl; }
         | PRINT NUMBER SCOLON       { driver.make_node<print_function<int>>($2); }
         | PRINT VAR SCOLON          { driver.make_node<print_function<std::string>>(std::move($2)); }
;

ctrl_statement:   IF OP_BRACK exp CL_BRACK OP_BRACE statement_block CL_BRACE {
                    $$ = driver.make_node<ctrl_statement>(CtrlStatement::IF, $3, $6);
                  }
                  | WHILE OP_BRACK exp CL_BRACK OP_BRACE statement_block CL_BRACE {
                    $$ = driver.make_node<ctrl_statement>(CtrlStatement::WHILE, $3, $6);
                  }
;

%%


void yy::parser::error(const location_type& l, const std::string &msg) {
  std::cout << "error pos: " << l << std::endl;
  throw std::runtime_error{msg};
}

