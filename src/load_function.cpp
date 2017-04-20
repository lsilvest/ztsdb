// (C) 2017 Leonardo Silvestri
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


#include "load_function.hpp"
#include "parser_ctx.hpp"
#include "anf.hpp"


// #define DEBUG

void core::loadFunctions(interp::BaseFrame* r) {

  ParserCtx pctx;

  auto res = pctx.parse(std::make_shared<std::string>
                        ("function(X, FUN) {"
                         "  l <- list() \n"
                         "  for (i in X) { l <- c(l, FUN(i)) } \n"
                         "  l }"));
  if (res == 0) {
    anf::convertToANF(pctx.prog.get());
    const auto f = static_cast<const Function*>(pctx.prog.get()->begin->e);
    r->add("lapply"s, val::Value(std::make_shared<val::VClos>(f)));
  }
}
