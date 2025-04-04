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
#include <cassert>

#include "ast_includes.hpp"

namespace yy {
  class scanner;
  class driver;
}

using namespace yy;
using namespace paracl::ast;

}

%code top {

#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <list>
#include <stack>

#include "driver.hpp"
#include "types.hpp"
#include "scanner.hpp"
#include "paracl_grammar.tab.hh"

static yy::parser::symbol_type yylex(yy::scanner &scanner) {
  return scanner.get_token();
}

}

%{
  using namespace paracl;
  using TypeID = PCLType::TypeID;
  
  std::mt19937 Generator(std::random_device{}());
  std::uniform_int_distribution<int> DistrInRange(INT_MIN, INT_MAX);
  std::stack<statement_block*> blocks;
  std::list<expression*> Accesses;
  std::list<expression*> PresetArrElems;
  bool IsRootBlockSet = false;
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
  ARRAY      "array"
  REPEAT     "repeat"
  UNDEF      "undef"
  SCAN       "?"
  OP_BRACK   "("
  CL_BRACK   ")"
  OP_BRACE   "{"
  CL_BRACE   "}"
  OP_SQBRACK   "["
  CL_SQBRACK   "]"
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

// terminal tokens
%token <int> NUMBER
%token <paracl::SymbNameType> VAR

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
%nterm <expression*>         array_expression
%nterm <UniformArray*>       uniform_array
%nterm <PresetArray*>        preset_array
%nterm <ArrayAccess*>        array_value
%nterm <ArrayHolder*>        array
%nterm <variable*>           lvalue_operand
%nterm <variable*>           variable

%nterm <expression*>         print_expression
%nterm <ctrl_statement*>     ctrl_statement

%left LESS LESS_EQ GREATER GREATER_EQ
%left EQ NEQ
%left LOGIC_AND LOGIC_OR
%nonassoc UPLUS UMINUS

%precedence THEN
%precedence ELSE
%precedence NEGATE

%start program

%%

program:
    statement_block {
        driver.set_ast_root(static_cast<root_statement_block*>($1));
    }
;

statement_block:
    %empty {
        if (!IsRootBlockSet) {
            blocks.push(driver.make_root_block());
            IsRootBlockSet = true;
        } else {
            blocks.push(driver.make_block());
        }
        $$ = blocks.top();
    }
    | statement_block statement {
        $1->add($2);
        $$ = $1;
    }
;

statement:
    OP_BRACE statement_block CL_BRACE {
        $$ = $2;
        blocks.pop();
        driver.change_scope(blocks.top());
    }
    | ctrl_statement {
        $$ = $1;
        blocks.pop();
        driver.change_scope(blocks.top());
    }
    | expression SCOLON { $$ = $1; }
    | SCOLON { $$ = driver.make_node<statement_block>(); }
;

variable:
    VAR {
        $$ = driver.make_node<variable>(
            blocks.top(), std::move($1), @$
        );
    }
;

lvalue_operand:
    variable { $$ = $1; }
    | array_value { $$ = $1; }
;

base_expression:
    OP_BRACK expression CL_BRACK { $$ = $2; }
    | NUMBER { $$ = driver.make_node<number>($1, @$); }
    | SCAN { $$ = driver.make_node<read_expression>(@$); }
    | lvalue_operand { $$ = $1; }
;

unary_expression:
    MINUS base_expression %prec UMINUS {
        $$ = driver.make_node<un_operator>(UnOp::MINUS, $2, @$);
    }
    | PLUS base_expression %prec UPLUS {
        $$ = driver.make_node<un_operator>(UnOp::PLUS, $2, @$);
    }
    | NEGATE base_expression %prec NEGATE {
        $$ = driver.make_node<un_operator>(UnOp::NEGATE, $2, @$);
    }
    | base_expression { $$ = $1; }
;

multiply_expression:
    multiply_expression MUL unary_expression {
        $$ = driver.make_node<calc_expression>(
            CalcOp::MUL, $1, $3, @$
        );
    }
    | multiply_expression DIV unary_expression {
        $$ = driver.make_node<calc_expression>(
            CalcOp::DIV, $1, $3, @$
        );
    }
    | multiply_expression PERCENT unary_expression {
        $$ = driver.make_node<calc_expression>(
            CalcOp::PERCENT, $1, $3, @$
        );
    }
    | unary_expression { $$ = $1; }
;

calc_expression:
    calc_expression PLUS multiply_expression {
        $$ = driver.make_node<calc_expression>(
            CalcOp::ADD, $1, $3, @$
        );
    }
    | calc_expression MINUS multiply_expression {
        $$ = driver.make_node<calc_expression>(
            CalcOp::SUB, $1, $3, @$
        );
    }
    | multiply_expression { $$ = $1; }
;

comparable_expression:
    comparable_expression LESS calc_expression {
        $$ = driver.make_node<logic_expression>(
            LogicOp::LESS, $1, $3, @$
        );
    }
    | comparable_expression LESS_EQ calc_expression {
        $$ = driver.make_node<logic_expression>(
            LogicOp::LESS_EQ, $1, $3, @$
        );
    }
    | comparable_expression GREATER calc_expression {
        $$ = driver.make_node<logic_expression>(
            LogicOp::GREATER, $1, $3, @$
        );
    }
    | comparable_expression GREATER_EQ calc_expression {
        $$ = driver.make_node<logic_expression>(
            LogicOp::GREATER_EQ, $1, $3, @$
        );
    }
    | calc_expression { $$ = $1; }
;

equality_expression:
    equality_expression EQ comparable_expression {
        $$ = driver.make_node<logic_expression>(
            LogicOp::EQ, $1, $3, @$
        );
    }
    | equality_expression NEQ comparable_expression {
        $$ = driver.make_node<logic_expression>(
            LogicOp::NEQ, $1, $3, @$
        );
    }
    | comparable_expression { $$ = $1; }
;

logical_expression:
    logical_expression LOGIC_AND equality_expression {
        $$ = driver.make_node<logic_expression>(
            LogicOp::AND, $1, $3, @$
        );
    }
    | logical_expression LOGIC_OR equality_expression {
        $$ = driver.make_node<logic_expression>(
            LogicOp::OR, $1, $3, @$
        );
    }
    | equality_expression { $$ = $1; }
;

assignment_expression:
    variable ASSIGN expression {
        $$ = driver.make_node<assignment>(
            blocks.top(), $1, $3, @$
        );
    }
    | variable ASSIGN array {
        $$ = driver.make_node<assignment>(
            blocks.top(), $1, $3, @$
        );
    }
    | array_value ASSIGN expression {
        $$ = driver.make_node<ArrayAccessAssignment>(
            blocks.top(), $1, $3, @$
        );
    }
;

expression:
    logical_expression { $$ = $1; }
    | assignment_expression { $$ = $1; }
    | print_expression { $$ = $1; }
;

print_expression:
    PRINT expression {
        $$ = driver.make_node<print_function>($2, @$);
    }
    | PRINT array {
        $$ = driver.make_node<print_function>($2, @$);
    }
;

array_value:
    VAR array_access {
        $$ = driver.make_node<ArrayAccess>(
            blocks.top(), std::move($1),
            Accesses.begin(), Accesses.end(), @$
        );
        Accesses.clear();
    }
;

array_access:
    OP_SQBRACK expression CL_SQBRACK array_access {
        Accesses.push_front($2);
    }
    | OP_SQBRACK expression CL_SQBRACK {
        Accesses.push_front($2);
    }
;

array:
    uniform_array {
        $$ = driver.make_node<ArrayHolder>(blocks.top(), $1, @$);
    }
    | preset_array {
        $$ = driver.make_node<ArrayHolder>(blocks.top(), $1, @$);
    }
;

uniform_array:
    REPEAT OP_BRACK array_expression COMMA expression CL_BRACK {
        $$ = driver.make_node<UniformArray>(
            blocks.top(), $3, $5, @$
        );
    }
;

array_expression:
    uniform_array { $$ = $1; }
    | preset_array { $$ = $1; }
    | expression { $$ = $1; }
    | UNDEF {
        $$ = driver.make_node<number>(DistrInRange(Generator), @$);
    }
;

preset_array:
    ARRAY OP_BRACK preset_array_access CL_BRACK {
        $$ = driver.make_node<PresetArray>(
            blocks.top(), PresetArrElems.begin(),
            PresetArrElems.end(), @$
        );
        PresetArrElems.clear();
    }
;

preset_array_access:
    array_expression COMMA preset_array_access {
        PresetArrElems.push_front($1);
    }
    | array_expression {
        PresetArrElems.push_front($1);
    }
    | %empty {}
;

ctrl_statement:
    WHILE OP_BRACK expression CL_BRACK statement {
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

%%

void yy::parser::error(const location_type &l, const std::string &msg) {
  std::cout << "error position: " << l << std::endl;
  throw std::runtime_error{msg};
}

