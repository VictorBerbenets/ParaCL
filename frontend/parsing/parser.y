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
  class driver;
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

static yy::parser::symbol_type yylex(yy::scanner &scanner, yy::driver &driver) {
  return scanner.get_token();
}

}

%param {yy::scanner& scanner}
%param {yy::driver& driver}

%define parse.trace
%define parse.error verbose
%define api.token.prefix {TOKEN_}

%token
  ASSIGN   "="
  MUL      "*"
  DIV      "/"
  PLUS     "+"
  MINUS    "-"
  EOF 0    "end of file"
  SCOLON   ";"
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
  EQ         "=="
  NEQ        "!="
  LESS       "<"
  LESS_EQ    "<="
  GREATER    ">"
  GREATER_EQ ">="

  LOGIC_AND  "&&"
  LOGIC_OR   "||"
;

// terminal tokens
%token <int> NUMBER
%token <std::string> VAR

%nterm <statement*>          statement
%nterm <statement_block*>    statement_block
%nterm <expression*>         expression
%nterm <logic_expression*>   logical_expression
%nterm <bin_operator*>       bin_operation
%nterm <un_operator*>        unary_operation
%nterm <definition*>         definition
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

program: statement_block { driver.set_ast_root($1); }
;

statement_block:  %empty { $$ = driver.make_node<statement_block>(); }
                | statement_block statement {
                    $1->add($2);
                    $$ = $1;
                  }
;

statement:  OP_BRACE statement_block CL_BRACE { $$ = $2; }
          | expression SCOLON                 { $$ = $1; }
          | ctrl_statement                    { $$ = $1; }
          | function                          { $$ = $1; }
          | definition                        { $$ = $1; }
;

expression:   logical_expression              { $$ = $1; }
            | bin_operation                   { $$ = $1; }
            | unary_operation                 { $$ = $1; }
            | OP_BRACK expression CL_BRACK    { $$ = $2; }
            | NUMBER                          { $$ = driver.make_node<number>($1);   }
            | VAR                             { $$ = driver.make_node<variable>(std::move($1)); }
;

unary_operation:   MINUS expression %prec UMINUS      { $$ = driver.make_node<un_operator>(UnOp::MINUS, $2); }
                 | PLUS  expression %prec UPLUS       { $$ = driver.make_node<un_operator>(UnOp::PLUS, $2);  }
;

bin_operation:   expression PLUS  expression   { $$ = driver.make_node<bin_operator>(BinOp::ADD, $1, $3); }
               | expression MINUS expression   { $$ = driver.make_node<bin_operator>(BinOp::SUB, $1, $3); }
               | expression MUL   expression   { $$ = driver.make_node<bin_operator>(BinOp::MUL, $1, $3); }
               | expression DIV   expression   { $$ = driver.make_node<bin_operator>(BinOp::DIV, $1, $3); }
;

logical_expression:   expression LESS expression        { $$ = driver.make_node<logic_expression>(LogicOp::LESS, $1, $3);       }
                    | expression LESS_EQ expression     { $$ = driver.make_node<logic_expression>(LogicOp::LESS_EQ, $1, $3);    }
                    | expression GREATER expression     { $$ = driver.make_node<logic_expression>(LogicOp::GREATER, $1, $3);    }
                    | expression GREATER_EQ expression  { $$ = driver.make_node<logic_expression>(LogicOp::GREATER_EQ, $1, $3); }
                    | expression EQ  expression         { $$ = driver.make_node<logic_expression>(LogicOp::EQ, $1, $3);         }
                    | expression NEQ expression         { $$ = driver.make_node<logic_expression>(LogicOp::NEQ, $1, $3);        }
                    | expression LOGIC_AND expression   { $$ = driver.make_node<logic_expression>(LogicOp::LOGIC_AND, $1, $3);  }
                    | expression LOGIC_OR  expression   { $$ = driver.make_node<logic_expression>(LogicOp::LOGIC_OR, $1, $3);   }
;

definition: VAR ASSIGN expression SCOLON    { $$ = driver.make_node<var_definition>(std::move($1), $3); }


function:  VAR ASSIGN SCAN SCOLON    { $$ = driver.make_node<scan_function>(std::move($1));  }
         | PRINT NUMBER SCOLON       { $$ = driver.make_node<print_function>($2);            }
         | PRINT VAR SCOLON          { $$ = driver.make_node<print_function>(std::move($2)); }
;

ctrl_statement:   IF OP_BRACK expression CL_BRACK OP_BRACE statement_block CL_BRACE {
                    $$ = driver.make_node<ctrl_statement>(CtrlStatement::IF, $3, $6);
                  }
                  | WHILE OP_BRACK expression CL_BRACK OP_BRACE statement_block CL_BRACE {
                    $$ = driver.make_node<ctrl_statement>(CtrlStatement::WHILE, $3, $6);
                  }
;

%%


void yy::parser::error(const location_type &l, const std::string &msg) {
  std::cout << "error pos: " << l << std::endl;
  throw std::runtime_error{msg};
}

