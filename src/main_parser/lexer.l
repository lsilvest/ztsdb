%{
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


#include <cerrno>
#include <climits>
#include <string>
#include <cstdlib>
#include "parser.hpp"
#include "../parser_ctx.hpp"
#include "../globals.hpp"
#include "../timezone/ztime.hpp"

typedef yy::parser::semantic_type YYSTYPE;
typedef yy::parser::location_type YYLTYPE;

#define yyterminate() return yy::parser::make_END(ctx.loc)
#define YY_DECL yy::parser::symbol_type yylex(ParserCtx& ctx, yyscan_t yyscanner)
YY_DECL;

// Code run each time a pattern is matched:
# define YY_USER_ACTION  ctx.loc.columns(yyleng);
%}


%option noyywrap never-interactive nounistd reentrant
%x str


d         [0-9]
h         [0-9a-fA-F]
a         [a-zA-Z\.\_]
exp       ([eE][+-]?{d}+)
empty     [ \t]
eol       [\n] 
comment   "#"[^\n]*
ign       {empty}*{comment}*{empty}*
basetime  {d}{4}-{d}{2}-{d}{2}[ ]+{d}{2}(:{d}{2}){2}(\.{d}{1,9})?([ ]+[a-zA-Z/_\-+0-9]+)?
dtime     \|\.[ ]*{basetime}[ ]*\.\|
interval  \|[+-]{1}[ ]*{basetime}[ ]*->[ ]*{basetime}[ ]*[+-]{1}\|
symbol    {a}({a}|{d})*

%%

%{
  ctx.loc.step();
%}

{eol}/({ign}{eol})+  { ctx.loc.lines(yyleng); ctx.loc.step(); }
{eol}      { ctx.loc.lines(yyleng); 
             ctx.loc.step(); 
             if (!ctx.ignore_nl) return yy::parser::make_STT(ctx.loc);
           }
{eol}/{ign}[)\]}]{1}  {
             ctx.loc.lines(yyleng); 
             ctx.loc.step(); 
             if (!ctx.ignore_nl) return yy::parser::make_STT(ctx.loc);
           }
{eol}/{ign},  {
             ctx.loc.lines(yyleng); 
             ctx.loc.step(); 
             if (!ctx.ignore_nl) return yy::parser::make_STT(ctx.loc);
           }
{eol}/{ign}else  {
             ctx.loc.lines(yyleng); 
             ctx.ignore_nl = true;
             ctx.loc.step(); 
           }              
{empty}+   { ctx.loc.step(); }
TRUE       { ctx.ignore_nl=false; return yy::parser::make_TRUE(ctx.loc); }
FALSE      { ctx.ignore_nl=false; return yy::parser::make_FALSE(ctx.loc); }
T          { ctx.ignore_nl=false; return yy::parser::make_TRUE(ctx.loc); }
F          { ctx.ignore_nl=false; return yy::parser::make_FALSE(ctx.loc); }
"&"        { ctx.ignore_nl=true; return yy::parser::make_AND(ctx.loc); }
"&&"       { ctx.ignore_nl=true; return yy::parser::make_AND2(ctx.loc); }
"|"        { ctx.ignore_nl=true; return yy::parser::make_OR(ctx.loc); }
"||"       { ctx.ignore_nl=true; return yy::parser::make_OR2(ctx.loc); }
"if"       { ctx.ignore_nl=true; return yy::parser::make_IF(ctx.loc); }
"else"     { ctx.ignore_nl=true; return yy::parser::make_ELSE(ctx.loc); }
"for"      { ctx.ignore_nl=true; return yy::parser::make_FOR(ctx.loc); }
"in"       { ctx.ignore_nl=true; return yy::parser::make_IN(ctx.loc); }
"next"     { ctx.ignore_nl=false; return yy::parser::make_NEXT(ctx.loc); }
"break"    { ctx.ignore_nl=false; return yy::parser::make_BREAK(ctx.loc); }
"while"    { ctx.ignore_nl=true; return yy::parser::make_WHILE(ctx.loc); }
"function" { ctx.ignore_nl=true; return yy::parser::make_FUNCTION(ctx.loc); }
"?"   { ctx.ignore_nl=true; return yy::parser::make_QUERY(ctx.loc); }
"("   { return yy::parser::make_LPAR(ctx.loc); }
")"   { ctx.ignore_nl=false; 
        return yy::parser::make_RPAR(ctx.loc); }
"{"   { ctx.ignore_nl=true; 
        return yy::parser::make_LCURLY(ctx.loc); }
"}"   { ctx.ignore_nl=false;
        return yy::parser::make_RCURLY(ctx.loc); }
"["   { ctx.ignore_nl=true; 
        return yy::parser::make_LSQUARE(ctx.loc); }
"]"   { ctx.ignore_nl=false; 
        return yy::parser::make_RSQUARE(ctx.loc); }
"[["  { ctx.ignore_nl=true; 
        return yy::parser::make_LLSQUARE(ctx.loc); }
"+"   { ctx.ignore_nl=true;  return yy::parser::make_PLUS(ctx.loc); }
"*"   { ctx.ignore_nl=true;  return yy::parser::make_MUL(ctx.loc); }
"-"   { ctx.ignore_nl=true;  return yy::parser::make_MINUS(ctx.loc); }
--    { ctx.ignore_nl=true;  return yy::parser::make_REF(ctx.loc); }
"++"  { ctx.ignore_nl=true;  return yy::parser::make_ESC(ctx.loc); }
"/"   { ctx.ignore_nl=true;  return yy::parser::make_DIV(ctx.loc); }
"%%"  { ctx.ignore_nl=true;  return yy::parser::make_MOD(ctx.loc); }
"^"   { ctx.ignore_nl=true;  return yy::parser::make_POWER(ctx.loc); }
"!"   { ctx.ignore_nl=false; return yy::parser::make_NOT(ctx.loc); }
">="  { ctx.ignore_nl=true;  return yy::parser::make_GE(ctx.loc); }
">"   { ctx.ignore_nl=true;  return yy::parser::make_GT(ctx.loc); }
"=="  { ctx.ignore_nl=true;  return yy::parser::make_EQ(ctx.loc); }
"!="  { ctx.ignore_nl=true;  return yy::parser::make_NE(ctx.loc); }
"<="  { ctx.ignore_nl=true;  return yy::parser::make_LE(ctx.loc); }
"<"   { ctx.ignore_nl=true;  return yy::parser::make_LT(ctx.loc); }
"<-"  { ctx.ignore_nl=true;  return yy::parser::make_LEFT_ASSIGN(ctx.loc); }
"="   { ctx.ignore_nl=true;  return yy::parser::make_EQ_ASSIGN(ctx.loc); }
"<<-" { ctx.ignore_nl=true;  return yy::parser::make_DBL_ASSIGN(ctx.loc); }
","   { ctx.ignore_nl=true;  return yy::parser::make_COMMA(ctx.loc); }
":"   { ctx.ignore_nl=true;  return yy::parser::make_COLON(ctx.loc); }
";"   { ctx.ignore_nl=true;  return yy::parser::make_STT(ctx.loc); }
"$"   { ctx.ignore_nl=true;  return yy::parser::make_DOLLAR(ctx.loc); }
"..." { ctx.ignore_nl=true;  return yy::parser::make_ELLIPSIS(ctx.loc); }
NULL  { ctx.ignore_nl=false; return yy::parser::make_NULL_(ctx.loc); }
Inf   { ctx.ignore_nl=false; return yy::parser::make_DOUBLE(Global::ZINF, ctx.loc); }
-Inf  { ctx.ignore_nl=false; return yy::parser::make_DOUBLE(-Global::ZINF, ctx.loc); }
NaN   { ctx.ignore_nl=false; return yy::parser::make_DOUBLE(Global::ZNAN, ctx.loc); }
`\+`  { ctx.ignore_nl=false; return yy::parser::make_OP(yy::parser::token::PLUS, ctx.loc); }
`-`   { ctx.ignore_nl=false; return yy::parser::make_OP(yy::parser::token::MINUS, ctx.loc); }
`\*`  { ctx.ignore_nl=false; return yy::parser::make_OP(yy::parser::token::MUL, ctx.loc); }
`\/`  { ctx.ignore_nl=false; return yy::parser::make_OP(yy::parser::token::DIV, ctx.loc); }
`\^`  { ctx.ignore_nl=false; return yy::parser::make_OP(yy::parser::token::POWER, ctx.loc); }
`\|`  { ctx.ignore_nl=false; return yy::parser::make_OP(yy::parser::token::OR, ctx.loc); }
`\|\|` { ctx.ignore_nl=false; return yy::parser::make_OP(yy::parser::token::OR2, ctx.loc); }
`&`   { ctx.ignore_nl=false; return yy::parser::make_OP(yy::parser::token::AND, ctx.loc); }
`&&`  { ctx.ignore_nl=false; return yy::parser::make_OP(yy::parser::token::AND2, ctx.loc); }
`>=`  { ctx.ignore_nl=false; return yy::parser::make_OP(yy::parser::token::GE, ctx.loc); }
`>`   { ctx.ignore_nl=false; return yy::parser::make_OP(yy::parser::token::GT, ctx.loc); }
`==`  { ctx.ignore_nl=false; return yy::parser::make_OP(yy::parser::token::EQ, ctx.loc); }
`\|=`  { ctx.ignore_nl=false; return yy::parser::make_OP(yy::parser::token::NE, ctx.loc); }
`<=`  { ctx.ignore_nl=false; return yy::parser::make_OP(yy::parser::token::LE, ctx.loc); }
`<`   { ctx.ignore_nl=false; return yy::parser::make_OP(yy::parser::token::LT, ctx.loc); }
`!`   { ctx.ignore_nl=false; return yy::parser::make_OP(yy::parser::token::NOT, ctx.loc); }
 

{comment}  { }
{d}+("."{d}*)?{exp}? { ctx.ignore_nl=false; 
                       return yy::parser::make_DOUBLE(std::atof(yyget_text(yyscanner)), ctx.loc); }
{symbol}             { ctx.ignore_nl=false; 
                       return yy::parser::make_SYMBOL(yyget_text(yyscanner), ctx.loc); }

{dtime}    { ctx.ignore_nl=false;
             try {
               return yy::parser::make_DTIME(tz::dtime_from_string(yyget_text(yyscanner), "|.%Y-%m-%d %H:%M:%S[.%s] %Z.|"), ctx.loc);
             }
             catch (const std::exception& e) {
               return yy::parser::make_ERROR("Error parsing dtime: "s + e.what(), ctx.loc);
             }
           }
{interval} { ctx.ignore_nl=false; 
             try {
               return yy::parser::make_INTERVAL(tz::interval_from_string(yyget_text(yyscanner)), ctx.loc); 
             }
             catch (const std::exception& e) {
               return yy::parser::make_ERROR("Error parsing interval: "s + e.what(), ctx.loc);
             }
           }


\"      ctx.string_literal = ""; BEGIN(str);

<str>\" { // closing quote
          BEGIN(INITIAL);
          ctx.ignore_nl=false; 
          return yy::parser::make_STRING(ctx.string_literal, ctx.loc);
        }

<str>\n {
          return yy::parser::make_ERROR("unterminated string constant: "s, ctx.loc);
        }

<str>\\[0-7]{1,3} { // octal escape sequence
          int result;
          (void) sscanf(yytext + 1, "%o", &result);
          if (result > 0xff) {
             return yy::parser::make_ERROR("octal constant out of bounds: "s, ctx.loc);
          }
          ctx.string_literal += result;
        }

<str>\\[0-9]+ {
          return yy::parser::make_ERROR("bad escape sequence: "s, ctx.loc);
        }

<str>\\n  ctx.string_literal += '\n';
<str>\\t  ctx.string_literal += '\t';
<str>\\r  ctx.string_literal += '\r';
<str>\\b  ctx.string_literal += '\b';
<str>\\f  ctx.string_literal += '\f';

<str>\\(.|\n)    ctx.string_literal += yytext[1]; 

<str>[^\\\n\"]+  ctx.string_literal += yytext;

