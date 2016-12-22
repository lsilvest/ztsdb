// (C) 2016 Leonardo Silvestri
//
// This file is part of ztsdb.
//
// ztsdb is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ztsdb is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ztsdb.  If not, see <http://www.gnu.org/licenses/>.


%language "C++"
%defines

// we use 10.1.5.2 Complete Symbols
%skeleton "lalr1.cc" /* -*- C++ -*- */
%define api.token.constructor
%define api.value.type variant
%define api.location.type {yy::location}
%define parse.assert

%code requires
{
#include <string>
#include <memory>
#include "location.hpp"
#include "../ast.hpp"
#include "../globals.hpp"
#include "../timezone/interval.hpp"
class ParserCtx;   // forward declaration to avoid circular dependency
typedef void *yyscan_t;
}

%code
{
#include <iostream>
#include "../parser_ctx.hpp"
#include "../cow_ptr.hpp"
#include "../array.hpp"

#define YY_DECL yy::parser::symbol_type yylex(ParserCtx& ctx, yyscan_t yyscanner)
YY_DECL;

  using namespace std;

}

// %destructor { delete $$; }  expr

%locations
%initial-action
{
  // Initialize the initial location.
  @$.begin.filename = @$.end.filename = ctx.file;
  @$.begin.s = @$.end.s = ctx.s;
};

%parse-param { ParserCtx &ctx }
%parse-param { yyscan_t scanner }
%lex-param { ParserCtx &ctx }
%lex-param { yyscan_t scanner }


%define parse.trace
%define parse.error verbose

// token declarations:
%token  <Global::dtime> DTIME
%token  <tz::interval>  INTERVAL
%token  <double>        DOUBLE 
%token  <string>        STRING 
%token  <string>        ERROR
%token  NULL_           "NULL"
%token  END        0    "EOF"
%token  ELLIPSIS        "..."
%token  FUNCTION        "function"
%token  <string>        SYMBOL 
%token  <unsigned>      OP 

%token  TRUE 
%token  FALSE

%token  LEFT_ASSIGN     "<-"
%token  DBL_ASSIGN      "<<-"
%token  EQ_ASSIGN       "="

%token  IF       "if"
%token  ELSE     "else"
%token  FOR      "for"
%token  IN       "in"
%token  WHILE    "while"
%token  NEXT     "next"
%token  BREAK    "break"

%token  EQ       "=="
%token  NE       "!="
%token  LT       "<"
%token  LE       "<="
%token  GT       ">"
%token  GE       ">="
%token  AND      "&"
%token  OR       "|"
%token  AND2     "&&"
%token  OR2      "||"
%token  DOLLAR   "$"
%token  PLUS     "+" 
%token  MINUS    "-" 
%token  MUL      "*"
%token  DIV      "/"
%token  MOD      "%%"
%token  POWER    "^"
%token  NOT      "!"

%token  LCURLY   "{"
%token  RCURLY   "}"
%token  LPAR     "(" 
%token  RPAR     ")"
%token  LSQUARE  "["
%token  RSQUARE  "]"
%token  LLSQUARE "[["

%token  COLON    ":"
%token  COMMA    ","
%token  QUERY    "?"

%token  STT      "; or EOL"
%token  REF      "--"
%token  ESC      "++"

// type declarations:
%type <E*>   expr
%type <El*>  exprlist
%type <El*>  prog
%type <El*>  actuals
%type <E*>   aarg
%type <El*>  formals
%type <E*>   farg

// precedence:
%right          EOA
%right          EXPR
%left           FUNCTION
%left           FOR WHILE
%nonassoc       IF
%nonassoc       ELSE
%nonassoc       EMPTY_ARG
%nonassoc       EMPTY_FUNCALL
%nonassoc       SUBSET DBLSUBSET FUNCALL
%nonassoc       SYMBOL OP
%right          LEFT_ASSIGN DBL_ASSIGN EQ_ASSIGN
%right          QUERY
%left           AND AND2
%left           OR OR2
%nonassoc       EQ NE LT LE GT GE 
%left           PLUS MINUS
%left           MUL  DIV  MOD
%left           COLON
%nonassoc       UNOT NOT UMINUS UPLUS
%nonassoc       REF ESC
%right          POWER
%left           DOLLAR
%nonassoc       LPAR LSQUARE LLSQUARE
%nonassoc       RPAR

%start prog

%%


// start point, returned via 'ctx.prog'
// -----------
prog    : exprlist[el]  { ctx.prog = std::unique_ptr<El>($el); }
        ;

// list of expressions
// -----------
exprlist:   expr[e]                     { $$ = new El($e); }
        |   exprlist[el] STT expr[e]    { $$ = ($el)->add($e); }
        |   exprlist[el] STT            { $$ = $el; }
        ;

// formal arguments to functions (function definition)
// -----------
formals:   farg[f]                          { $$ = new El($f); }
        |  formals[fl] COMMA farg[f]        { $$ = ($fl)->add($f); }
        ;

farg    :   SYMBOL[s]                       { $$ = new Symbol($s, @$); }
        |   ELLIPSIS                        { $$ = new Ellipsis(@$); }
        |   SYMBOL[s] EQ_ASSIGN expr[e]     { $$ = new TaggedExpr(new Symbol($s, @s), $e, @$); }
        ;

// actual arguments to functions (includes subsetting)
// -----------
actuals :   aarg[a]                                 { $$ = new El($a); }
        |   actuals[as] COMMA aarg[a]               { $$ = ($as)->add($a); }
        ;

aarg    :   %prec EMPTY_ARG                      { $$ = new Null(@$); }
        |   REF SYMBOL[s]                        { $$ = new Symbol($s, @$, true);   }       
        |   expr[e]                              { $$ = $e; }
        |   SYMBOL[s1] EQ_ASSIGN REF SYMBOL[s2]  { $$ = new TaggedExpr(new Symbol($s1, @s1), new Symbol($s2, @s2, true), @$); }
        |   SYMBOL[s]  EQ_ASSIGN expr[e]         { $$ = new TaggedExpr(new Symbol($s, @s), $e, @$); }
        |   STRING[s1] EQ_ASSIGN REF SYMBOL[s2]  { $$ = new TaggedExpr(new Symbol($s1, @s1), new Symbol($s2, @s2, true), @$); }
        |   STRING[s]  EQ_ASSIGN expr[e]         { $$ = new TaggedExpr(new Symbol($s, @s), $e, @$); }
        ;

// expressions
// -----------
expr    :       LPAR expr[e]        RPAR         { $$ = $e;           }
        |       LCURLY              RCURLY       { $$ = new Null(@$); }
        |       LCURLY exprlist[el] RCURLY       { $$ = $el;          }

// control structures
        |       IF LPAR expr[c] RPAR expr[e] %prec IF                { $$ = new IfElse($c, $e, new Null(yy::missing_loc()), @$); }
        |       IF LPAR expr[c] RPAR expr[e1] ELSE expr[e2]          { $$ = new IfElse($c, $e1, $e2, @$); }
        |       FOR LPAR SYMBOL[s] IN expr[i] RPAR expr[b] %prec FOR { $$ = new For(new Symbol($s, @s), $i, $b, @$); }
        |       WHILE LPAR expr[c] RPAR expr[b] %prec WHILE          { $$ = new While($c, $b, @$); }
        |       NEXT                                                 { $$ = new Next(@$);  }
        |       BREAK                                                { $$ = new Break(@$); }

// escape expression
        |       ESC   expr[e] %prec ESC           { $$ = new Escape($e, @$); }

// query operator
        |       expr[e1] QUERY          expr[e2]  { $$ = new Request($e1, $e2, @$);  }

// assign
        |       expr[e1] LEFT_ASSIGN    expr[e2]  { $$ = new LeftAssign($e1, $e2, @$);  }
        |       expr[e1] DBL_ASSIGN     expr[e2]  { $$ = new SpecialAssign($e1, $e2, @$);  }

// symbol and constants
        |       SYMBOL          { $$ = new Symbol($1, @$);   }       
        |       ERROR[e]        { ctx.error(@e, $e); YYERROR; }
        |       DOUBLE          { $$ = new Double($1, @$); }
        |       STRING          { $$ = new String($1, @$);   }
        |       NULL_           { $$ = new Null(@$);       }
        |       DTIME           { $$ = new Dtime($1, @$);    }
        |       INTERVAL        { $$ = new Interval($1, @$); }
        |       TRUE            { $$ = new Bool(true, @$);   }
        |       FALSE           { $$ = new Bool(false, @$);  }                      

// unary operators (including function call style (i.e. `+`(x))
        |       MINUS expr[e] %prec UMINUS       { $$ = new Unop(parser::token::MINUS, $e, @$); }
        |       PLUS  expr[e] %prec UPLUS        { $$ = new Unop(parser::token::PLUS,  $e, @$); }
        |       NOT   expr[e] %prec UNOT         { $$ = new Unop(parser::token::NOT,   $e, @$); }
        |       OP[op] LPAR expr[e] RPAR         { $$ = new Unop($op, $e, @$); }
        |       OP[op] LPAR REF SYMBOL[s] RPAR   { $$ = new Unop($op, new Symbol($s, @s, true), @$); }

// binary operators (including function call style (i.e. `+`(x, y) and `+`(x, y, tz))
        |       expr[e1] PLUS  expr[e2]  { $$ = new Binop(parser::token::PLUS,  $e1, $e2, @$); }
        |       expr[e1] MINUS expr[e2]  { $$ = new Binop(parser::token::MINUS, $e1, $e2, @$); }
        |       expr[e1] MUL   expr[e2]  { $$ = new Binop(parser::token::MUL,   $e1, $e2, @$); }
        |       expr[e1] DIV   expr[e2]  { $$ = new Binop(parser::token::DIV,   $e1, $e2, @$); }
        |       expr[e1] MOD   expr[e2]  { $$ = new Binop(parser::token::MOD,   $e1, $e2, @$); }
        |       expr[e1] POWER expr[e2]  { $$ = new Binop(parser::token::POWER, $e1, $e2, @$); }
        |       expr[e1] COLON expr[e2]  { $$ = new Binop(parser::token::COLON, $e1, $e2, @$); }
        |       expr[e1] EQ    expr[e2]  { $$ = new Binop(parser::token::EQ,    $e1, $e2, @$); }
        |       expr[e1] NE    expr[e2]  { $$ = new Binop(parser::token::NE,    $e1, $e2, @$); }
        |       expr[e1] LT    expr[e2]  { $$ = new Binop(parser::token::LT,    $e1, $e2, @$); } 
        |       expr[e1] LE    expr[e2]  { $$ = new Binop(parser::token::LE,    $e1, $e2, @$); }
        |       expr[e1] GE    expr[e2]  { $$ = new Binop(parser::token::GE,    $e1, $e2, @$); }
        |       expr[e1] GT    expr[e2]  { $$ = new Binop(parser::token::GT,    $e1, $e2, @$); } 
        |       expr[e1] AND   expr[e2]  { $$ = new Binop(parser::token::AND,   $e1, $e2, @$); } 
        |       expr[e1] AND2  expr[e2]  { $$ = new Binop(parser::token::AND2,  $e1, $e2, @$); }
        |       expr[e1] OR    expr[e2]  { $$ = new Binop(parser::token::OR,    $e1, $e2, @$); }
        |       expr[e1] OR2   expr[e2]  { $$ = new Binop(parser::token::OR2,   $e1, $e2, @$); }
        |       OP[op] LPAR expr[e1] COMMA expr[e2] RPAR { $$ = new Binop($op,  $e1, $e2, @$); }
        |       OP[op] LPAR expr[e1] COMMA expr[e2] COMMA expr[e3] RPAR { $$ = new Binop($op, $e1, $e2, @$, $e3); }
        |       OP[op] LPAR REF SYMBOL[s1] COMMA expr[e2] RPAR { $$ = new Binop($op, new Symbol($s1, @s1, true), $e2, @$); }
        |       OP[op] LPAR REF SYMBOL[s1] COMMA expr[e2] COMMA expr[e3] RPAR { $$ = new Binop($op, new Symbol($s1, @s1, true), $e2, @$, $e3); }

// function definition:
        |       FUNCTION LPAR formals[fl] RPAR expr[e] %prec FUNCTION { $$ = new Function($fl, $e, @$); }
        |       FUNCTION LPAR formals[fl] RPAR STT expr[e] %prec FUNCTION { $$ = new Function($fl, $e, @$); }
        |       FUNCTION LPAR RPAR expr[e] %prec FUNCTION { $$ = new Function($e, @$); }
        |       FUNCTION LPAR RPAR STT expr[e] %prec FUNCTION { $$ = new Function($e, @$); }

// function call
        |       expr[e]   LPAR actuals[sl] RPAR %prec FUNCALL { $$ = new Funcall($e, $sl); }
        |       SYMBOL[s] LPAR actuals[sl] RPAR %prec FUNCALL { $$ = new Funcall(new Symbol($s, @s), $sl); }
        |       SYMBOL[s] LPAR actuals[sl] RPAR LEFT_ASSIGN expr[e] { $$ = new Funcall(new Symbol($s, @s), ($sl)->add($e)); }
        |       expr[e] LPAR RPAR      %prec EMPTY_FUNCALL    { $$ = new Funcall($e, new El()); }
        |       SYMBOL[s] LPAR RPAR    %prec EMPTY_FUNCALL    { $$ = new Funcall(new Symbol($s, @s), new El()); }

// subset and subassign
//
//     Operators [] (subset), [<- (subassign), [[]] (dblsubset), [[<- (dblsubassign) are converted
//     to function calls with the symbol or expression on which the operation is taking place as
//     first argument. Treating them as functions has a performance penalty, but it simplifies the
//     interpreter considerably.
        |       expr[e]   LSQUARE actuals[sl] RSQUARE %prec SUBSET { $$ = new Funcall(new Symbol("subset", @$), ($sl)->addfirst($e)); }
        |       SYMBOL[s] LSQUARE actuals[sl] RSQUARE %prec SUBSET { $$ = new Funcall(new Symbol("subset", @$), ($sl)->addfirst(new Symbol($s, @s))); }
        |       expr[e]   LSQUARE actuals[sl] RSQUARE LEFT_ASSIGN expr[e2] { $$ = new Funcall(new Symbol("subassign", @$), 
                                                                                                   (($sl)->addfirst($e2))->addfirst($e)); }
//     Symbol subassign is a special case because when subsetting symbols, the symbol itself must be
//     modified, so the 'Symbol' created must have its ref flag set to true.
        |       SYMBOL[s] LSQUARE actuals[sl] RSQUARE LEFT_ASSIGN expr[e2] { $$ = new Funcall(new Symbol("subassign", yy::missing_loc()), 
                                                                                                   (($sl)->addfirst($e2))->addfirst(new Symbol($s, @s, true))); }
        |       expr[e]   LLSQUARE actuals[sl] RSQUARE RSQUARE %prec DBLSUBSET { $$ = new Funcall(new Symbol("dblsubset", yy::missing_loc()), 
                                                                               ($sl)->addfirst($e)); }
        |       SYMBOL[s] LLSQUARE actuals[sl] RSQUARE RSQUARE %prec DBLSUBSET { $$ = new Funcall(new Symbol("dblsubset", yy::missing_loc()), 
                                                                               ($sl)->addfirst(new Symbol($s, @s))); }
        |       expr[e]   LLSQUARE actuals[sl] RSQUARE RSQUARE LEFT_ASSIGN expr[e2] { $$ = new Funcall(new Symbol("dblsubassign", yy::missing_loc()), 
                                                                                                                    (($sl)->addfirst($e2))->addfirst($e)); }
        |       SYMBOL[s] LLSQUARE actuals[sl] RSQUARE RSQUARE LEFT_ASSIGN expr[e2] { $$ = new Funcall(new Symbol("dblsubassign", yy::missing_loc()), 
                                                                                                      (($sl)->addfirst($e2))->addfirst(new Symbol($s, @s, true))); }
        |       expr[e] DOLLAR SYMBOL[s] %prec SUBSET { $$ = new Funcall(new Symbol("dblsubset", yy::missing_loc()), 
                                                                    (new El(new String($s, @s)))->addfirst($e)); }
        |       expr[e] DOLLAR STRING[s] %prec SUBSET { $$ = new Funcall(new Symbol("dblsubset", yy::missing_loc()), 
                                                                    (new El(new String($s, @s)))->addfirst($e)); }
        |       SYMBOL[s1] DOLLAR SYMBOL[s2] %prec SUBSET { $$ = new Funcall(new Symbol("dblsubset", yy::missing_loc()), 
                                                                    (new El(new String($s2, @s2)))->addfirst(new Symbol($s1, @s1))); }
        |       SYMBOL[s1] DOLLAR STRING[s2] %prec SUBSET { $$ = new Funcall(new Symbol("dblsubset", yy::missing_loc()), 
                                                                    (new El(new String($s2, @s2)))->addfirst(new Symbol($s1, @s1))); }
        |       expr[e] DOLLAR SYMBOL[s] LEFT_ASSIGN expr[e2]  { $$ = new Funcall(new Symbol("dblsubassign", yy::missing_loc()), 
                                                                                         (new El(new String($s, @s)))->addfirst($e2)->addfirst($e)); }
        |       SYMBOL[s1] DOLLAR SYMBOL[s2] LEFT_ASSIGN expr[e2]  { $$ = new Funcall(new Symbol("dblsubassign", yy::missing_loc()), 
                                                                                         (new El(new String($s2, @s2)))->addfirst($e2)->addfirst(new Symbol($s1, @s1, true))); }
        |       expr[e] DOLLAR STRING[s] LEFT_ASSIGN expr[e2]  { $$ = new Funcall(new Symbol("dblsubassign", yy::missing_loc()), 
                                                                                         (new El(new String($s, @s)))->addfirst($e2)->addfirst($e)); }
        |       SYMBOL[s1] DOLLAR STRING[s2] LEFT_ASSIGN expr[e2]  { $$ = new Funcall(new Symbol("dblsubassign", yy::missing_loc()), 
                                                                                         (new El(new String($s2, @s2)))->addfirst($e2)->addfirst(new Symbol($s1, @s1, true))); }
        ;

%%


void yy::parser::error (const location_type& l, const std::string& m)
{
  ctx.error(l, m);
}
