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
%define api.location.file "location.hh"

%code requires {

#include <iostream>
#include <string>
#include <stdexcept>
#include <utility>

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
#include <stack>

#include "driver.hpp"
#include "scanner.hpp"
#include "paracl_grammar.tab.hh"

static yy::parser::symbol_type yylex(yy::scanner &scanner) {
  return scanner.get_token();
}

}

%{
  std::stack<statement_block*> blocks;
%}

%param {yy::scanner& scanner}
%parse-param {yy::driver& driver}

%define parse.trace
%define parse.error verbose
%define api.token.prefix {TOKEN_}

%token
  ASSIGN   "="
  MUL      "*"
  DIV      "/"
  PLUS     "+"
  MINUS    "-"
  PERCENT  "%"
  EOF 0    "end of file"
  SCOLON   ";"
  COMMA    ","
;

// statement tokens
%token
  IF         "if"
  ELSE       "else"
  WHILE      "while"
  PRINT      "print"
  SCAN       "?"
  OP_BRACK   "("
  CL_BRACK   ")"
  OP_BRACE   "{"
  CL_BRACE   "}"
  OPSQ_BRACK "["
  CLSQ_BRACK "]"
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

  NEGATE     "!"
;

// array tokens
%token
  REPEAT "repeat"
  UNDEF  "undef"
  ARRAY  "array"
;

// terminal tokens
%token <int> NUMBER
%token <std::string> NAME

// nterminals

%nterm <statement*>          statement
%nterm <statement_block*>    statement_block
%nterm <expression*>         expression
%nterm <expression*>         base_expression
%nterm <expression*>         logical_expression
%nterm <expression*>         calc_expression
%nterm <expression*>         unary_expression
%nterm <expression*>         multiply_expression
%nterm <expression*>         equality_expression
%nterm <expression*>         comparable_expression
%nterm <expression*>         assignment_expression
%nterm <expression*>         has_value
%nterm <object_type*>        array_elem
%nterm <array*>              array_type
%nterm <function*>           function
%nterm <ctrl_statement*>     ctrl_statement

%nterm <std::vector<expression*>>  array_brackets

%left LESS LESS_EQ GREATER GREATER_EQ
%left EQ NEQ
%left LOGIC_AND LOGIC_OR
%nonassoc UPLUS UMINUS

%precedence THEN
%precedence ELSE
%precedence NEGATE

%start program

%%

program: statement_block { driver.set_ast_root($1); }
;

statement_block:  %empty {
                          blocks.push(driver.make_block());
                          $$ = blocks.top();
                  }
                | statement_block statement {
                    $1->add($2);
                    $$ = $1;
                  }
;

statement:  OP_BRACE statement_block CL_BRACE {
              $$ = $2;
              blocks.pop();
              driver.change_scope(blocks.top());
            }
          | ctrl_statement {
              $$ = $1;
              blocks.pop();
              driver.change_scope(blocks.top());
            }
          | expression SCOLON                 { $$ = $1; }
          | function                          { $$ = $1; }
          | SCOLON                            { $$ = driver.make_node<statement_block>(); }
;

base_expression:  OP_BRACK expression CL_BRACK    { $$ = $2; }
                | has_value                       { $$ = $1; }
;

has_value:  NUMBER     { $$ = driver.make_node<integer_literal>($1, @$); }
          | NAME       { $$ = driver.make_node<integer_variable>($1, blocks.top(), @$); }
          | array_elem { $$ = $1; }

unary_expression:   MINUS  base_expression %prec UMINUS   { $$ = driver.make_node<un_operator>(UnOp::MINUS, $2, @$);  }
                  | PLUS   base_expression %prec UPLUS    { $$ = driver.make_node<un_operator>(UnOp::PLUS, $2, @$);   }
                  | NEGATE base_expression %prec NEGATE   { $$ = driver.make_node<un_operator>(UnOp::NEGATE, $2, @$); }
                  | SCAN                                  { $$ = driver.make_node<read_expression>(@$); }
                  | base_expression                       { $$ = $1; }
;

multiply_expression:   multiply_expression MUL     unary_expression   { $$ = driver.make_node<calc_expression>(CalcOp::MUL, $1, $3, @$);     }
                     | multiply_expression DIV     unary_expression   { $$ = driver.make_node<calc_expression>(CalcOp::DIV, $1, $3, @$);     }
                     | multiply_expression PERCENT unary_expression   { $$ = driver.make_node<calc_expression>(CalcOp::PERCENT, $1, $3, @$); }
                     | unary_expression                               { $$ = $1;}
;

calc_expression: calc_expression PLUS    multiply_expression   { $$ = driver.make_node<calc_expression>(CalcOp::ADD, $1, $3, @$); }
               | calc_expression MINUS   multiply_expression   { $$ = driver.make_node<calc_expression>(CalcOp::SUB, $1, $3, @$); }
               | multiply_expression                           { $$ = $1; }
;

comparable_expression:   comparable_expression LESS calc_expression        { $$= driver.make_node<logic_expression>(LogicOp::LESS, $1, $3, @$);        }
                       | comparable_expression LESS_EQ calc_expression     { $$ = driver.make_node<logic_expression>(LogicOp::LESS_EQ, $1, $3, @$);    }
                       | comparable_expression GREATER calc_expression     { $$ = driver.make_node<logic_expression>(LogicOp::GREATER, $1, $3, @$);    }
                       | comparable_expression GREATER_EQ calc_expression  { $$ = driver.make_node<logic_expression>(LogicOp::GREATER_EQ, $1, $3, @$); }
                       | calc_expression                                   { $$ = $1; }
;

equality_expression:  equality_expression EQ  comparable_expression        { $$ = driver.make_node<logic_expression>(LogicOp::EQ, $1, $3, @$);  }
                    | equality_expression NEQ comparable_expression        { $$ = driver.make_node<logic_expression>(LogicOp::NEQ, $1, $3, @$); }
                    | comparable_expression                                { $$ = $1; }


logical_expression:   logical_expression LOGIC_AND equality_expression   { $$ = driver.make_node<logic_expression>(LogicOp::LOGIC_AND, $1, $3, @$);  }
                    | logical_expression LOGIC_OR  equality_expression   { $$ = driver.make_node<logic_expression>(LogicOp::LOGIC_OR, $1, $3, @$);   }
                    | equality_expression                                { $$ = $1; }
;

assignment_expression: NAME ASSIGN assignment_expression {
                         //auto int_var = driver.make_node<integer_variable>($1,
                         //blocks.top(), @1);
                         $$ = driver.make_node<integer_variable>($1,
                         blocks.top(), $3, @$);
                       }
                     | NAME ASSIGN logical_expression    {
                         //auto int_var = driver.make_node<integer_variable>($1, blocks.top(),  @1);
                         $$ = driver.make_node<integer_variable>($1,
                         blocks.top(), $3, @$);
                       }
                     | array_elem ASSIGN expression      {}
                     | array_type                        { $$ = $1; }
;

expression:   logical_expression    { $$ = $1; }
            | assignment_expression { $$ = $1; }
;

function:  PRINT expression SCOLON   { $$ = driver.make_node<print_function>($2, @$); }
;

ctrl_statement: WHILE OP_BRACK expression CL_BRACK  statement {
                    blocks.push(driver.make_block());
                    driver.change_scope(blocks.top());
                    $$ = driver.make_node<while_operator>($3, $5, @$);
                }
              | IF OP_BRACK expression CL_BRACK statement %prec THEN {
                  blocks.push(driver.make_block());
                  driver.change_scope(blocks.top());
                  $$ = driver.make_node<if_operator>($3, $5, @$);
              }
              | IF OP_BRACK expression CL_BRACK statement ELSE statement {
                  blocks.push(driver.make_block());
                  driver.change_scope(blocks.top());
                  $$ = driver.make_node<if_operator>($3, $5, $7, @$);
                }
;

array_elem: NAME array_brackets { $$ =
          driver.make_node<array_elem>($1, blocks.top(), $2, @$); }
;
 
array_brackets: array_brackets OPSQ_BRACK has_value CLSQ_BRACK {
                  $1.push_back($3);
                  $$ = $1;
                }
              | OPSQ_BRACK has_value CLSQ_BRACK {
                  $$.push_back($2);
                }
;

array_type:  NAME ASSIGN REPEAT OP_BRACK base_expression COMMA base_expression CL_BRACK {
               $$ = driver.make_node<array>($1, blocks.top(), $5, $7, @$);
             }
           | NAME ASSIGN REPEAT OP_BRACK UNDEF COMMA base_expression CL_BRACK  {std::cout << "?\n";}
           | NAME ASSIGN REPEAT OP_BRACK UNDEF COMMA SCAN CL_BRACK  {  }
           | NAME ASSIGN ARRAY  OP_BRACK initializer_list CL_BRACK {}
;

initializer_list:   has_value COMMA initializer_list
                  | has_value
;

%%

void yy::parser::error(const location_type &l, const std::string &msg) {
  std::cout << "error position: " << l << std::endl;
  throw std::runtime_error{msg};
}

