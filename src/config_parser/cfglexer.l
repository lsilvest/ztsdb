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
#include "../config_ctx.hpp"
#include "cfgparser.hpp"


//# define YY_USER_ACTION yylloc->columns (yyleng);
typedef cfgyy::parser::semantic_type YYSTYPE;
typedef cfgyy::parser::location_type YYLTYPE;

#define yyterminate() return cfgyy::parser::make_END(ctx.yylloc);

static int64_t tolong(const char* b) {
  char* endptr;
  return std::strtol(b, &endptr, 0);
}

using cprs = cfgyy::parser;

// a bunch of the definitions below come from:
// http://www.quut.com/c/ANSI-C-grammar-l.html
%}

%option noyywrap never-interactive nounistd reentrant prefix="cfgyy"
%x str

comment  #[^\n]*
O        [0-7]
D        [0-9]
NZ       [1-9]
L        [a-zA-Z_]
A        [a-zA-Z_0-9.]
H        [a-fA-F0-9]
HP       (0[xX])
E        ([Ee][+-]?{D}+)
P        ([Pp][+-]?{D}+)
FS       (f|F|l|L)
IS       (((u|U)(l|L|ll|LL)?)|((l|L|ll|LL)(u|U)?))
CP       (u|U|L)
SP       (u8|u|U|L)
ES       (\\(['"\?\\abfnrtv]|[0-7]{1,3}|x[a-fA-F0-9]+))
WS       [ \t\v\f]
eol      [\n] 

%%
{comment}  {  }
{WS}       {  }
{eol}      { ctx.yylloc.lines(yyleng); ctx.yylloc.step();  return cprs::make_NL(ctx.yylloc); }
=          { return cprs::make_EQ(ctx.yylloc); }

{HP}{H}+{IS}?      { return cprs::make_INTEGER(tolong(yyget_text(yyscanner)), ctx.yylloc); }
-?{NZ}{D}*{IS}?    { return cprs::make_INTEGER(tolong(yyget_text(yyscanner)), ctx.yylloc); }
"0"{O}*{IS}?       { return cprs::make_INTEGER(tolong(yyget_text(yyscanner)), ctx.yylloc); }
{CP}?"'"([^'\\\n]|{ES})+"'"  { return cprs::make_INTEGER(tolong(yyget_text(yyscanner)), ctx.yylloc); }

-?{D}+{E}{FS}?            { return cprs::make_DOUBLE(std::atof(yyget_text(yyscanner)), ctx.yylloc); }
-?{D}*"."{D}+{E}?{FS}?    { return cprs::make_DOUBLE(std::atof(yyget_text(yyscanner)), ctx.yylloc); }
-?{D}+"."{E}?{FS}?        { return cprs::make_DOUBLE(std::atof(yyget_text(yyscanner)), ctx.yylloc); }
{HP}{H}+{P}{FS}?        { return cprs::make_DOUBLE(std::atof(yyget_text(yyscanner)), ctx.yylloc); }
{HP}{H}*"."{H}+{P}{FS}? { return cprs::make_DOUBLE(std::atof(yyget_text(yyscanner)), ctx.yylloc); }
{HP}{H}+"."{P}{FS}?     { return cprs::make_DOUBLE(std::atof(yyget_text(yyscanner)), ctx.yylloc); }

{A}+ { return cprs::make_SYMBOL(yyget_text(yyscanner), ctx.yylloc); }


\"      ctx.string_literal = ""; BEGIN(str);

<str>\" { /* saw closing quote - all done */
          BEGIN(INITIAL);
          return cprs::make_STRING_LITERAL(ctx.string_literal.c_str(), ctx.yylloc);
        }
<str>\\a { ctx.string_literal += '\a'; }
<str>\\b { ctx.string_literal += '\b'; }
<str>\\f { ctx.string_literal += '\f'; }
<str>\\n { ctx.string_literal += '\n'; }
<str>\\r { ctx.string_literal += '\r'; }
<str>\\t { ctx.string_literal += '\t'; }
<str>\\v { ctx.string_literal += '\v'; }
<str>\\[0-7]{1,3} { auto s = yyget_text(yyscanner);
                    unsigned char c = 0;
                    for (size_t i=1; i<strlen(s); ++i) { c = 8 * c + (s[i] - '0'); }
                    ctx.string_literal += c; }
<str>\\x[a-fA-F0-9]+ { auto s = yyget_text(yyscanner);
                       unsigned char c = 0;
                       for (unsigned i=2; i<strlen(s); ++i) { 
                         c = 16 * c + (toupper(s[i]) - (toupper(s[i]) <= '9' ? '0' : '7')); }
                       std::cout << "c is: " << long(c) << std::endl;
                       ctx.string_literal += c; }

<str>([^"\\\n])*   { ctx.string_literal += std::string(yyget_text(yyscanner)); }
