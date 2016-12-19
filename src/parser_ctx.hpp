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


#ifndef PARSER_CTX_HPP
#define PARSER_CTX_HPP


#include "location.hpp"
#include "ast.hpp"


class ParserCtx {
public:
  ParserCtx();

  // Run the parser on file F.
  // Return 0 on success.
  int parsefile(const std::string& filename);
  int parse(const std::shared_ptr<const std::string> s_p);

  /// Error handling routine called by the parser in case of
  /// error. Saves the string and location for subsequent use when
  /// building an error message.
  void error(const yy::location& l, const std::string& m);

  ~ParserCtx();

  // The name of the file being parsed.
  // Used later to pass the file name to the location tracker.
  std::string file;

  /// Whether scanner/parser traces should be generated.
  bool trace_scanning;
  bool trace_parsing;

  bool ignore_nl;     
  std::unique_ptr<El> prog;

  yy::location loc;
  /// Data structure for the treatment of strings literals that
  /// require context sensitive lexing.
  std::string string_literal;

  std::shared_ptr<const std::string> s;

  yy::location errorLoc;
  std::string errorString;
 
};


#endif
