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


#include "valuevar.hpp"
#include "interp_error.hpp"
#include "interp_ctx.hpp"
#include "parser_ctx.hpp"
#include "anf.hpp"


val::VBuiltinG::VBuiltinG(interp::BaseFrame* r,
                          string name, 
                          string signature_s, 
                          function<Value(vector<arg_t>&, 
                                         zcore::InterpCtx& ic)> f_p,
                          bool evalEllipsis_p,
                          map<string, ArgInfo> argInfo_p) : 
  f(f_p), invoke(make_shared<Invoke>(name, yy::missing_loc())), 
  evalEllipsis(evalEllipsis_p), argInfo(argInfo_p)
{
  assert(name.length() < MAXNAME);
  ParserCtx pctx;
#ifdef DEBUG
  pctx.trace_scanning = true;
  pctx.trace_parsing  = true;
#endif
  if (pctx.parse(std::make_shared<const std::string>(signature_s)) == 0) {
    anf::convertToANF(pctx.prog.get());
    signature = shared_ptr<Function>(static_cast<Function*>(pctx.prog->begin->e));
    pctx.prog->remove_nodelete(pctx.prog->begin);
    signature->processFormlist(argMap, ellipsisPos);
  }
  else {
    throw std::range_error("error parsing builtin signature: " + name + " <- " + signature_s);
  }
  r->add(name, *this);
}


val::Value val::VBuiltinG::operator()(interp::BuiltinFrame& a, zcore::InterpCtx& ic) const 
{ 
  return f(a.mv, ic); 
}


/// Check that the arguments are of the correct type.
void val::VBuiltinG::checkArgs(const interp::BuiltinFrame& r) const {
  for (const auto& e : r.mv) {
    const auto& info = argInfo.find(val::getName(e));
    if (info != argInfo.end() && info->second.doEval) {
      auto& typeset = info->second.typeset;
      if (typeset.empty()) {
        throw out_of_range("empty typeset for parameter " + get<0>(e));
      }
      const auto& val = val::getVal(e);
      if (typeset.count(static_cast<val::ValType>(val.which())) == 0) {
        std::ostringstream ss;
        for (auto i = typeset.begin(); i != typeset.end(); ) {
          ss << '\'' << vt_to_string.at(*i) << '\'';
          if (++i != typeset.end()) {
            ss << ',';
          }
        } 
        throw interp::EvalException("invalid argument type: '" + val::getName(e) 
                                    + "' should be any of " + ss.str()
                                    + " but is '" + vt_to_string.at(val.which()) + '\'',
                                    val::getLoc(e));
      }
    }
  }
}

