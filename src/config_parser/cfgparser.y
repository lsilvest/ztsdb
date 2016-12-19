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
%define parse.assert

%expect 1

%code requires
{
#include <string>
#include "../config.hpp"
namespace cfg {
  struct CfgCtx;   // forward declaration to avoid circular dependency
}
typedef void *yyscan_t;
}

%code
{
#include <iostream>
#include "../config_ctx.hpp"
}


%locations
%initial-action
{
  // Initialize the initial location.
  @$.begin.filename = @$.end.filename = &ctx.file;
};

%parse-param { cfg::CfgCtx& ctx }
%parse-param { yyscan_t scanner }
%lex-param { cfg::CfgCtx& ctx }
%lex-param { yyscan_t scanner }
%define api.prefix {cfgyy}

%define parse.trace
%define parse.error verbose



%token          END       0  "end of file"
%token          <std::string> STRING_LITERAL
%token          <std::string> SYMBOL
%token          <int64_t> INTEGER
%token          <double> DOUBLE
%token          EQ
%token          NL


%type <cfg::configmap_t>  cfg
%type <std::pair<std::string, cfg::CfgVariant>>  kv

%start cfg

%%


cfg  : %empty
     | kv[e] NL      { ctx.add($e); }
     | cfg kv[e] NL  { ctx.add($e); }
     | cfg NL        { }
     ;

kv   : SYMBOL[s] EQ STRING_LITERAL[sl]   { $$ = make_pair($s, cfg::CfgVariant($sl)); }
     | SYMBOL[s] EQ SYMBOL[sr]           { $$ = make_pair($s, cfg::CfgVariant($sr)); }
     | SYMBOL[s] EQ INTEGER[i]           { $$ = make_pair($s, cfg::CfgVariant($i)); }
     | SYMBOL[s] EQ DOUBLE[d]            { $$ = make_pair($s, cfg::CfgVariant($d)); }
     ;

%%


void cfgyy::parser::error(const cfgyy::parser::location_type& l, const std::string& m)
{
  ctx.error(l, m);
}
