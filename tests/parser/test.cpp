// -*- compile-command: "make -j -k test" -*-

// Copyright (C) 2015 Leonardo Silvestri
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


#include <crpcut.hpp>
#include "parser_ctx.hpp"
#include "anf.hpp"
#include "timezone/zone.hpp"


tz::Zones tzones("/usr/share/zoneinfo");            // LLL this is platform dependent...


// create encoder and use the parser to create the expression
static void parse(const char* buf, E*& eout) {
  ParserCtx pctx;
  //pctx.trace_parsing  = true;
  if (pctx.parse(std::make_shared<const std::string>(buf)) != 0) {
    // fail the section or report an error LLL
    cerr << "parser error" << endl;
  }
  eout = pctx.prog->clone();
}


static E* eout;


TEST(ast_parse_if_else) {
  parse("if (T) { a;\n b; c}\n"
        "else\n"
        "c\n"
        , eout);
  std::cout << to_string(*eout) << std::endl;
  delete eout;
}

int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}

