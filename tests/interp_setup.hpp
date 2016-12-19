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


// This header sets up a fully functional interpreter so that language
// features may be tested.

#include <crpcut.hpp>
#include "parser_ctx.hpp"
#include "interp.hpp"
#include "interp_error.hpp"
#include "msg_handler.hpp"
#include "env.hpp"
#include "anf.hpp"
#include "load_builtin.hpp"
#include "timezone/zone.hpp"
#include "timezone/ztime.hpp"
#undef INFO
#include "logging.hpp"


tz::Zones tzones("/usr/share/zoneinfo");
zlog::Logger lg;
cfg::CfgMap cfg::cfgmap;


using namespace interp;

// create encoder and use the parser to create the expression
static std::unique_ptr<E> parse(const char* buf) {
  ParserCtx pctx;
  //pctx.trace_parsing  = true;
  if (pctx.parse(std::make_shared<const std::string>(buf)) != 0) {
    // fail the section or report an error LLL
    cerr << "parser error" << endl;
  }
  anf::convertToANF(pctx.prog.get());
  return std::move(pctx.prog);
}


shpfrm base    = make_shared<Frame>("base"s);
shpfrm global  = make_shared<Frame>("global"s,  global, base);


val::Value eval(const unique_ptr<E>& ein) {
  cfg::cfgmap.set("timezone", "America/New_York");
  core::loadBuiltinFunctions(base.get());
  shpfrm evalEnv = make_shared<Frame>("working"s, global, global);
  zcore::MsgHandlerBase ir;
  zcore::InterpCtxRemote ic(ir, global);
  vector<shpfrm> fstack;
  auto halt = make_shared<Kont>(Kont{nullptr, nullptr, evalEnv, nullptr, Kont::NORMAL});
  auto k = make_shared<Kont>(Kont{nullptr, ein.get(), evalEnv, halt, Kont::NORMAL});
  auto is = zcore::InterpState{0, 0, 0,
                               std::unique_ptr<E>(ein->clone()),
                               k,
                               vector<shpfrm>{evalEnv},
                               std::map<Global::reqid_t, zcore::RspState>()};
  ic.s = &is;
  while (is.k->next) {
    is.k = step(is.k, fstack, ic);
  }
  evalEnv->clearTmp();
  cout << "evalEnv: " << endl;
  cout << string(*evalEnv) << endl;
  if (is.k->var) {
    if (is.k->var->etype == etsymbol) {
      auto s = static_cast<const Symbol*>(is.k->var);
      return evalEnv->find(s->data);
    }
    else {
      throw range_error("interp_setup.hpp::eval: not implemented");
    }
  } else {
    return evalEnv->find(".Last.value");
  }
}
