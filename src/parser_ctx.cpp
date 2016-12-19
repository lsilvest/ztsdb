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


#include <fstream>
#include <sstream>
#include "parser.hpp"
#include "parser_ctx.hpp"
#include "lexer.hpp"

ParserCtx::ParserCtx () : trace_scanning (false), trace_parsing(false), ignore_nl(true),
                          prog(nullptr)
{  }


ParserCtx::~ParserCtx () { }


int ParserCtx::parsefile(const std::string& filename) {
  file = filename;
  std::ifstream t(filename);
  if (t.fail()) {
    throw range_error(strerror(errno));
  }
  std::stringstream buffer;
  buffer << t.rdbuf();
  return parse(make_shared<const std::string>(buffer.str())); // unfortunately, can't move here
}


int ParserCtx::parse(const std::shared_ptr<const std::string> s_p)
{
  s = s_p;                      // keep it so it can be accessed by
                                // the parser and given to the nodes
                                // of the AST
  loc.initialize(file, 1u, 1u, s_p);

  yyscan_t scanner;     
  yylex_init(&scanner);
  yyset_debug(trace_scanning, scanner);
  //scan_begin(scanner);
  YY_BUFFER_STATE state = yy_scan_string(s->c_str(), scanner);

  yy::parser p(*this, scanner);
  p.set_debug_level (trace_parsing);
  
  int res = p.parse();

  yy_delete_buffer(state, scanner);
  yylex_destroy(scanner);

  //scan_end(scanner);
  return res;
}


void ParserCtx::error(const yy::location& l, const std::string& m)
{
  errorLoc = l;
  errorString = m;
}
