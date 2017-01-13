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


#include <sys/wait.h>
#include <fstream>
#include "base_funcs.hpp"
#include "parser_ctx.hpp"
#include "anf.hpp"
#include "interp_ctx.hpp"
#include "timezone/ztime.hpp"
#include "interp_error.hpp"
#include "interp.hpp"
#include "base_funcs.hpp"
#include "stats.hpp"
#include "info.hpp"
#include "config.hpp"
#include "logging.hpp"

extern zlog::Logger lg;

using namespace interp;


static function<string(string)> gensym = GensymFun();


/// These are a set of builtin functions. They are located here
/// because they need access to the continuation.

/// Read in a source file. The file is parsed and the result of the
/// parsing is inserted as the next continuation.
val::Value funcs::source(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  ParserCtx pctx;
#if 1 //def DEBUG
  pctx.trace_scanning = true;
  pctx.trace_parsing  = true;
#else
  pctx.trace_scanning = false;
  pctx.trace_parsing  = false;
#endif
  const auto& filename = val::get_scalar<arr::zstring>(getVal(v[0]));
  int res;
  try {
    res = pctx.parsefile(filename);
  }
  catch (const std::exception& e) {
    throw interp::EvalException("cannot read file '"s + filename + "': " + e.what(), getLoc(v[0]));
  }
  if (res == 0) {
    anf::convertToANF(pctx.prog.get());
#ifdef DEBUG    
    cout << "ANF is: " << to_string(*pctx.prog) << endl;
#endif
    // need to add the source code to the next environment, so that it
    // does not get deleted until it has finished executing:
    ic.s->k->next->r->add(gensym("?source"), 
                       val::VCode(std::shared_ptr<E>(static_cast<E*>(pctx.prog.get()))));

    auto el = static_cast<const El*>(pctx.prog.release());
    static const Null null(yy::missing_loc());
    // 'next->next' below is to get beyond the sentinel Kont
    // introduced by 'applyBuiltin':
    ic.s->k->next = make_shared<interp::Kont>(interp::Kont{nullptr, 
          &null, 
          ic.s->k->next->next->r,
          buildElChain(el->begin, el->n, ic.s->k->next->next->r, ic.s->k->next->next),
          interp::Kont::NORMAL});
  }
  else {
    throw interp::EvalException(pctx.errorString, pctx.errorLoc);
  }

  return val::VNull();
}


static BaseFrame* getEnv(shpfrm env, val::Value envir, yy::location envirloc) {
  auto envStack = env->getStack();
  if (envir.which() == val::vt_string) {
    const string ename = val::get_scalar<arr::zstring>(envir);
    if (ename == "current") {
      return envStack[0];
    }
    else {
      auto iter = std::find_if (envStack.begin(), envStack.end(),
                                [&ename](auto e) { return e->name == ename; });
      if (iter == envStack.end()) {
        throw interp::EvalException("can't find environment "s + ename, envirloc);      
      }
      return *iter;
    }
  }
  else {
    int pos = val::get_scalar<double>(envir);
    if (pos == -1) {
      return envStack[0];
    }
    else {
      --pos;
      if (pos >= 0 && static_cast<unsigned>(pos) < envStack.size()) {
        return envStack[pos];
      }
      else {
        throw interp::EvalException("environment position out of bounds", envirloc);
      }
    }
  }  
}


/// Find a variable in the specified environment.
val::Value funcs::getvar(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { X, ENVIR, INHERIT };

  const string x = val::get_scalar<arr::zstring>(getVal(v[X]));
  const bool inherits = val::get_scalar<bool>(getVal(v[INHERIT]));
  const auto env = getEnv(ic.s->k->r->up, getVal(v[ENVIR]), getLoc(v[ENVIR]));
  return inherits ? env->find(x) : env->findLocal(x);
} 

/// Build a list of variable names in the specified environment.
val::Value funcs::ls(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  const auto env = getEnv(ic.s->k->r->up, getVal(v[0]), getLoc(v[0]));
  return env->getNames();
} 


/// Assign a variable to a variable in the specified environment.
val::Value funcs::assign(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { X, VALUE, ENVIR, INHERIT };

  const string x = val::get_scalar<arr::zstring>(getVal(v[X]));
  auto value = getVal(v[VALUE]);
  const bool inherit = val::get_scalar<bool>(getVal(v[INHERIT]));

  auto env = getEnv(ic.s->k->r->up, getVal(v[ENVIR]), getLoc(v[ENVIR]));

  if (inherit) {
    env->addSpecial(x, std::move(value));
  }
  else {
    env->add(x, std::move(value));
  }
  return getVal(v[VALUE]);
}


/// Remove a variable from the specified environment.
val::Value funcs::rm(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { LIST, ENVIR, INHERIT };
  const auto& list = get<val::SpVAS>(getVal(v[LIST]));
  const bool inherit = val::get_scalar<bool>(getVal(v[2]));

  auto env = getEnv(ic.s->k->r->up, getVal(v[ENVIR]), getLoc(v[ENVIR]));
  
  // eliminate any variable/string present in the ellipsis:
  for (size_t i=3; i<v.size(); ++i) {
    // it's a VCode (for sure, since it's not evaluated) in which
    // there is either a symbol or a string
    const auto& code = get<val::VCode>(getVal(v[i]));
    auto e = code.expr.get();
    if (e->etype == etexprlist) {
      e = (static_cast<const El*>(e))->begin->e;
    }
    if (e->etype == ettaggedexpr) {
      e = (static_cast<const TaggedExpr*>(e))->e;
    }
    if (e->etype == etsymbol) {
      const auto s = static_cast<const Symbol*>(e);
      auto res = inherit ? env->removeSpecial(s->data) : env->remove(s->data);
      if (!res) {
        throw interp::EvalException("object '"s + s->data + "' not found"s, s->loc);
      }
    }
    else if (e->etype == etstring) {
      const auto s = static_cast<const String*>(e);
      const string& ss = val::get_scalar<arr::zstring>(s->data);
      auto res = inherit ? env->removeSpecial(ss) : env->remove(ss);
      if (!res) {
        throw interp::EvalException("object '"s + ss + "' not found"s, s->loc);
      }
    }
    else {
      throw interp::EvalException("... must contain names or character strings", e->loc);
    }
  }

  // eliminate any variable/string present in list 
  for (size_t i=0; i<list->size(); ++i) {
    auto res = inherit ? env->removeSpecial((*list)[i]) : env->remove((*list)[i]);
    if (!res) {
      throw std::out_of_range("object '"s +  (*list)[i] + "' not found"s);
    }
  }

  ic.s->k->next->next->atype |= interp::Kont::SILENT;
  return val::VNull();
} 


val::Value funcs::tryCatch(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  auto expr = get<val::VCode>(ic.s->k->r->find("expr"));
  auto catchcode = get<val::VCode>(ic.s->k->r->find("catch"));

#ifdef DEBUG    
  cout << "tryCatch, expr:  " << to_string(*expr.expr) << endl;
  cout << "tryCatch, catch: " << to_string(*catchcode.expr) << endl;
  cout << "tryCatch: ic.s->k->r:" << ic.s->k->r << endl;
  cout << "tryCatch: ic.s->k->next->next->r:" << ic.s->k->next->next->r << endl;
  cout << "tryCatch: ic.s->k->r->up:" << ic.s->k->r->up << endl;
#endif

  ic.s->fstack.push_back(std::make_shared<ShadowFrame>(ic.s->k->r->up, nullptr, nullptr, nullptr));
  auto r = ic.s->fstack.back();

  // we need to add a copy in the 'up' frame or else they will be
  // deleted before we can use them (when ic->k->r is destroyed):
  r->up->add(gensym("?expr"), expr);
  r->up->add(gensym("?catchcode"), catchcode);

  // we are in the middle of function invocation, so ic.s->k->r is the
  // builtin function environment (of tryCatch); we set the escape
  // continuation to catchcode and set the next continuation to expr:
  // put a sentinel continuation to make sure fenv is cleared:
  auto ksentinel = make_shared<Kont>(Kont{nullptr, nullptr, r, ic.s->k->next, Kont::END});
  r->ec = 
    make_shared<Kont>(Kont{nullptr, catchcode.expr.get(), r, ksentinel, Kont::NORMAL});
  ic.s->k->next = 
    make_shared<Kont>(Kont{nullptr, expr.expr.get(), r, ksentinel, Kont::NORMAL});

  return val::VNull();
} 


val::Value funcs::do_call(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  auto what = get<1>(v[0]);      // String, VBuiltinG or VClos!
  auto args = get<val::SpVList>(get<1>(v[1])); // VList

  auto el = new El();
  for (size_t j=0; j<args->size(); ++j) {
    auto elt = args->a[j];
    const auto& eltname = args->a.getnames(0)[j];
    if (eltname == "") {
      const auto tmpname = gensym("?arg");
      ic.s->k->next->r->add(tmpname, move(elt));
      el->add(new Symbol(tmpname, yy::missing_loc())); // think about loc LLL
    }
    else {
      ic.s->k->next->r->add(eltname, move(elt));
      el->add(new TaggedExpr(new Symbol(eltname, yy::missing_loc()), 
                             new Symbol(eltname, yy::missing_loc()), 
                             yy::missing_loc()));
    }
  }

  bool isstring = what.which() == val::vt_string;
  if (!isstring) {
    // add the function or builtin to the environment:
    ic.s->k->r->add("?function", move(what));
  }
  auto f = std::make_shared<Funcall>(new Symbol(isstring ? 
                                                    val::get_scalar<arr::zstring>(what) : 
                                                    "?function", yy::missing_loc()), 
                                         el);
  auto vf = val::VCode(f);
  ic.s->k->next = make_shared<Kont>(Kont{nullptr, f.get(), ic.s->k->r, ic.s->k->next, Kont::NORMAL});

  // we need to add 'f' to the next frame so that the expression list
  // gets properly destroyed:
  ic.s->k->next->r->add(gensym("?f"), std::move(vf));
  
  return val::VNull();
}


val::Value funcs::print(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { X, TZ };
  // if we have a time, interval or zts we take into account argument
  // TZ in order to allow printout in a defined time-zone:
  const std::string tz = val::get_scalar<arr::zstring>(getVal(v[TZ]));
  if (tz.length()) {
    static cfg::CfgMap localCfg;
    localCfg.set("timezone", tz);

    switch (getVal(v[X]).which()) {
    case val::vt_time: {
      const auto& a = get<val::SpVADT>(getVal(v[X]));
      cout << val::display(*a, a->getnames(0).names, localCfg) << endl;
    }
    break;
    case val::vt_interval: {
      const auto& a = get<val::SpVAIVL>(getVal(v[X]));
      cout << val::display(*a, a->getnames(0).names, localCfg) << endl;
    }
    break;
    case val::vt_zts: {
      const auto& a = get<val::SpZts>(getVal(v[X]));
      cout << val::display(a->getArray(), a->getIndex().getcol(0), localCfg) << endl;
    }
    break;
    default:
      cout << val::display(getVal(v[X])) << endl;
    }
  }
  else {
    cout << val::display(getVal(v[X])) << endl;
  }      
  ic.s->k->next->next->atype |= interp::Kont::SILENT; // while () while () will not work? LLL
  return getVal(v[X]);
}


template<typename T>
static val::Value names_get(const arr::Array<T>& a, const arr::idx_type d) {
  // build as a list of vectors
  if (d >= a.dim.size()) {
    if (std::any_of(a.names.begin(), a.names.end(), [](const auto& nm) { return nm->hasNames(); })) {
      auto  l = arr::make_cow<val::VList>(false, arr::Array<val::Value>(arr::idx_type{0}));
      for (arr::idx_type i=0; i<a.dim.size(); ++i) {
        if (a.hasNames(i)) {
          auto nd = arr::make_cow<arr::Array<arr::zstring>>(false, arr::idx_type{0});
          const auto names = a.getNames(i);
          for (arr::idx_type j=0; j<names.size(); ++j) {
            nd->concat(names[j], "");
          }
          l->push_back(make_pair("", nd));
        }
        else {
          l->push_back(make_pair("", val::VNull()));
        }           
      }
      return l;
    }
    else {
      // no dimension has any names:
      return val::VNull();
    }
  }
  else {
    if (a.hasNames(d)) {
      auto nd = arr::make_cow<arr::Array<arr::zstring>>(false, arr::idx_type{0});
      const auto names = a.getNames(d);
      for (arr::idx_type j=0; j<names.size(); ++j) {
        nd->concat(names[j], "");
      }    
      return nd;
    }
    else {
      return val::VNull();
    }
  }
}

  
template<typename T>
static void names_set(arr::Array<T>& a, 
                      const val::VList& l, 
                      const arr::idx_type d) {
  for (arr::idx_type i=0; i<l.size(); ++i) {
    const auto& e = l.a[i];
    switch (e.which()) {
    case val::vt_string: {
      const auto& s = get<val::SpVAS>(e);
      a.names[i] = make_unique<arr::Dname>(a.dim[i], Vector<zstring>(s->getcol(0)));
      break;
    }
    case val::vt_null:
      a.names[i] = make_unique<arr::Dname>(a.dim[i]);
      break;
    default:
      throw std::out_of_range("names must be of type 'character'");

    }
  }
}


template<typename T>
static void names_set(arr::Array<T>& a, 
                      const val::VArrayS& s, 
                      const arr::idx_type d) {
  a.names[d] = make_unique<arr::Dname>(a.dim[d], Vector<zstring>(*s.v[0]));
}


static val::Value names_get_hl(const val::Value& v0, arr::idx_type d) {
  switch (v0.which()) {
  case val::vt_zts:
    return names_get(get<val::SpZts>(v0)->getArray(), d);
  case val::vt_double:
    return names_get(*get<val::SpVAD>(v0), d);
  case val::vt_list:
    return names_get(get<val::SpVList>(v0)->a, 0);
  case val::vt_bool: 
    return names_get(*get<val::SpVAB>(v0), d);
  case val::vt_time: 
    return names_get(*get<val::SpVADT>(v0), d);
  case val::vt_duration:
    return names_get(*get<val::SpVADUR>(v0), d);
  case val::vt_interval:
    return names_get(*get<val::SpVAIVL>(v0), d);
  case val::vt_string:
    return names_get(*get<val::SpVAS>(v0), d);
  default:
    return val::VNull();           // like R
  }
}


template<typename T>
static void names_set_hl(const val::Value& v0, const T& l, arr::idx_type d) {
  switch (v0.which()) {
  case val::vt_zts: {
    auto z = get<val::SpZts>(v0);
    names_set(*z.get()->getArrayPtr(), l, d);
    break;
  }
  case val::vt_double: {
    auto a = get<val::SpVAD>(v0);
    names_set(*a.get(), l, d);
    break;
  }
  case val::vt_list: {
    auto a = get<val::SpVAD>(v0);
    names_set(*a.get(), l, d);
    break;
  }
  case val::vt_bool: {
    auto a = get<val::SpVAB>(v0);
    names_set(*a.get(), l, d);
    break;
  }
  case val::vt_time: {
    auto a = get<val::SpVADT>(v0);
    names_set(*a.get(), l, d);
    break;
  }
  case val::vt_duration: {
    auto a = get<val::SpVADUR>(v0);
    names_set(*a.get(), l, d);
    break;
  }
  case val::vt_interval: {
    auto a = get<val::SpVAIVL>(v0);
    names_set(*a.get(), l, d);
    break;
  }
  case val::vt_string: {
    auto a = get<val::SpVAS>(v0);
    names_set(*a.get(), l, d);
    break;
  }
  default:
    // in R:  "target of assignment expands to non-language object"
    throw std::out_of_range("invalid target of assignment");
  }
}


template<arr::idx_type D>
static val::Value names(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  if (v.size() == 1) {
    return names_get_hl(get<1>(v[0]), D);
  }
  else {
    ic.s->k->next->next->atype |= interp::Kont::SILENT;
    if (get<1>(v[1]).which() == val::vt_list) {
      names_set_hl(get<1>(v[0]), *get<val::SpVList>(get<1>(v[1])), D);
      return get<1>(v[0]);
    }
    else if (get<1>(v[1]).which() == val::vt_string) {
      names_set_hl(get<1>(v[0]), *get<val::SpVAS>(get<1>(v[1])), D);
      return get<1>(v[0]);
    }
    else {
      throw std::out_of_range("names must be of type 'character'");
    }
  }
}


val::Value funcs::colnames(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return names<1>(v, ic);
}


val::Value funcs::rownames(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return names<0>(v, ic);
}


val::Value funcs::dimnames(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return names<std::numeric_limits<arr::idx_type>::max()>(v, ic);
}


val::Value funcs::system(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  // system(command, intern = FALSE,
  //      ignore.stdout = FALSE, ignore.stderr = FALSE,
  //      wait = TRUE, input = NULL, show.output.on.console = TRUE,
  //      minimized = FALSE, invisible = TRUE)
  string command     = val::get_scalar<arr::zstring>(get<1>(v[0]));
  auto intern        = val::get_scalar<bool>(get<1>(v[1]));
  auto ignore_stdout = val::get_scalar<bool>(get<1>(v[2]));
  auto ignore_stderr = val::get_scalar<bool>(get<1>(v[3]));
  auto dowait        = val::get_scalar<bool>(get<1>(v[4]));

  // set up a pipe so the parent can read the child's output
  int link[2];
  if (pipe(link) < 0) {
    throw std::system_error(std::error_code(errno, std::system_category()), 
                            "funcs::system(): pipe");
  }

  // The child will inherit STDOUT and STDERR, so we need to flush
  // here to be sure we don't get mixed parent and child I/O:
  std::cout << flush;
  std::cerr << flush;

  pid_t pid = fork();
  if (pid < 0) {
    throw std::system_error(std::error_code(errno, std::system_category()), 
                            "funcs::system(): fork");
  }

  // child -------------------------------------------------
  if (pid == 0) {
    if (intern) {
      dup2 (link[1], STDOUT_FILENO);
      dup2 (link[1], STDERR_FILENO);
    }
    else {
      if (ignore_stdout) {
        int dn = open("/dev/null", O_RDWR);
        if (dup2(dn, STDOUT_FILENO) < 0) {
          throw std::system_error(std::error_code(errno, std::system_category()), 
                                  "funcs::system(): dup2 STDOUT_FILENO");

        }
        close(dn);
      }
      if (ignore_stderr) {
        int dn = open("/dev/null", O_RDWR);
        if (dup2(dn, STDERR_FILENO) < 0) {
          throw std::system_error(std::error_code(errno, std::system_category()), 
                                  "funcs::system(): dup2 STDERR_FILENO");

        }
        close(dn);
      }
    }
    close(link[0]);
    close(link[1]);
    fflush(stdout);
    execlp("bash", "bash", "-c", command.c_str(), nullptr);
    exit(EXIT_SUCCESS);         // silence the compiler
  }
  // parent ------------------------------------------------
  else {
    close(link[1]);
    if (intern) {
      auto a = make_cow<val::VArrayS>(false, rsv, arr::Vector<arr::idx_type>{0}); 
      char buf[1024]; buf[0] = 0;
      int offset = 0;
      for (;;) {
        ssize_t nbytes = read(link[0], buf+offset, sizeof(buf)-offset-1);
        buf[offset+nbytes] = 0;
        if (nbytes < 0) {
          throw std::system_error(std::error_code(errno, std::system_category()), 
                                  "funcs::system() - read");
        }
        else if (nbytes == 0) {
          if (offset == 0) {
            return a;
          }
          else {
            // take care of last element (which does not have EOL):
            a->concat(arr::zstring(buf));
            close(link[1]);
            return a;
          }
        }
        char *saveptr;
        char* tok = strtok_r(buf, "\n", &saveptr);
        if (tok) {
          for (;;) {
            char* ntok = strtok_r(NULL, "\n", &saveptr);
            if (ntok) {
              // tok was not last item in buf
              a->concat(arr::zstring(tok));
              tok = ntok;
            }
            else {
              // tok was last item buf
              memmove(buf, tok, strnlen(tok, sizeof(buf)-1)+1); // memmove as it can be overlapping
              offset = strnlen(buf, sizeof(buf));
              break;            // go back to reading
            }
          }
        }
        else {
          close(link[1]);
          return val::make_array(arr::zstring(""));
        }
      }
      waitpid(pid, nullptr, 0);
      return val::make_array(arr::zstring(buf)); 
    }
    else {                      // not intern
      if (dowait) {
        waitpid(pid, nullptr, 0);
      }
      fflush(stdout);
      close(link[1]);
      ic.s->k->next->next->atype |= interp::Kont::SILENT;
      return val::make_array(0.0);
    }
  }
}


val::Value funcs::stats_msg(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  const auto& stats = ic.getMsgStats();
  // create a vector with the obtained values:
  auto a = arr::make_cow<arr::Array<double>>
    (false,
     arr::Vector<arr::idx_type>{10,1},
     arr::Vector<double>{
       static_cast<double>(stats.bytesOutREQ),      
       static_cast<double>(stats.bytesOutRSP),      
       static_cast<double>(stats.bytesInREQ),       
       static_cast<double>(stats.bytesInRSP),
       static_cast<double>(stats.bytesAppend),
       static_cast<double>(stats.bytesAppendVector),
       static_cast<double>(stats.nbInREQ),          
       static_cast<double>(stats.nbInRSP),          
       static_cast<double>(stats.nbAppend),         
       static_cast<double>(stats.nbAppendVector)   
     }, 
     std::vector<arr::Vector<arr::zstring>> {
       {
         "bytes outgoing req",
         "bytes outgoing rsp",
         "bytes incoming req",
         "bytes incoming rsp",
         "bytes append", 
         "bytes vector append",
         "nb incoming req buffers",
         "nb incoming rsp buffers",
         "nb append", 
         "nb vector append"
       }, 
       {"value"}
     }
     );
  auto reset = val::get_scalar<bool>(getVal(v[0]));
  if (reset) {
    ic.resetMsgStats();
  }
  return a;
}


val::Value funcs::stats_net(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  const auto& stats = ic.getNetStats();
  // create a vector with the obtained values:
  auto a = arr::make_cow<arr::Array<double>>
    (false,
     arr::Vector<arr::idx_type>{13,1}, 
     arr::Vector<double>{
       static_cast<double>(stats.nbInConn),       
       static_cast<double>(stats.nbOutConn),      
       static_cast<double>(stats.nbCloseInConn),  
       static_cast<double>(stats.nbCloseOutConn), 
       static_cast<double>(stats.nbOutBuffers),            
       static_cast<double>(stats.nbSendFail),            
       static_cast<double>(stats.nbInBuffers),
       static_cast<double>(stats.nbInBuffersDrop),
       static_cast<double>(stats.bytesTimedOut),  
       static_cast<double>(stats.nbFailInCtx),          
       static_cast<double>(stats.nbReadFail),           
       static_cast<double>(stats.nbInMalformed),
       static_cast<double>(stats.readbuflistmax)
     }, 
     std::vector<arr::Vector<arr::zstring>> {
       {
         "nb incoming connections",
         "nb outgoing connections",      
         "nb close incoming connections",   
         "nb close outgoing connections",
         "nb outgoing buffers", 
         "nb send fail", 
         "nb incoming buffers",
         "nb incoming buffers dropped",
         "nb bytes timedout",  
         "nb context not found",
         "nb read fail",      
         "nb incoming segments malformed",
         "max nb of queued buffers"
       }, 
       {"value"}
     }
     );

  auto reset = val::get_scalar<bool>(getVal(v[0]));
  if (reset) {
    ic.resetNetStats();
  }
  return a;
}


val::Value funcs::stats_ctx(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  const auto& stats = ic.getCtxStats();
  // create a vector with the obtained values:
  auto a = arr::make_cow<arr::Array<double>>
    (false,
     arr::Vector<arr::idx_type>{12,1}, 
     arr::Vector<double>{
       static_cast<double>(stats.nbOutREQ),         
       static_cast<double>(stats.nbOutRSP),         
       static_cast<double>(stats.nbInREQ),          
       static_cast<double>(stats.nbInRSP),          
       static_cast<double>(stats.nbAppend),         
       static_cast<double>(stats.nbAppendVector),   
       static_cast<double>(stats.bytesOutREQ),      
       static_cast<double>(stats.bytesOutRSP),      
       static_cast<double>(stats.bytesInREQ),       
       static_cast<double>(stats.bytesInRSP),       
       static_cast<double>(stats.bytesAppend),      
       static_cast<double>(stats.bytesAppendVector)
     },
     std::vector<arr::Vector<arr::zstring>> {
       {
         "nb outgoing req",
         "nb outgoing rsp",
         "nb incoming req",
         "nb incoming rsp",
         "nb append", 
         "nb vector append",
         "bytes outgoing req",
         "bytes outgoing rsp",
         "bytes incoming req",
         "bytes incoming rsp",
         "bytes append", 
         "bytes vector append"
       }, 
       {"value"}
     }
     );

  auto reset = val::get_scalar<bool>(getVal(v[0]));
  if (reset) {
    ic.resetCtxStats();
  }
  return a;
}


// provide a first approximation of the info (can be extended)
val::Value funcs::info_net(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  // the info we have is:
  // - the connections
  // - the fdToId mapping
  // - readybuflist
  // - bufmap size
  // - signalling list
  // but see 'NetInfo' for what we actually implement:
  auto netinfo = ic.getNetInfo();
  auto connections = arr::make_cow<val::VArrayS>
    (false,
     arr::Vector<arr::idx_type>{0,4}, 
     arr::Vector<arr::zstring>{},
     std::vector<arr::Vector<arr::zstring>> {
       { }, 
       {"connection_id", "address", "port", "direction"}
     }
     );
  for (auto& c: netinfo.conninfo) {
    connections->rbind
      (val::VArrayS({1,4}, 
                    {std::to_string(c.id), c.ipaddress, std::to_string(c.port), 
                    c.dir == zcore::ConnectionInfo::Direction::INCOMING ? 
                        "incoming" : "outgoing"}));
  }
  auto buffering = arr::make_cow<val::VArrayD>
    (false,
     arr::Vector<arr::idx_type>{2,1}, 
     arr::Vector<double>{ static_cast<double>(netinfo.buflistSz), 
                          static_cast<double>(netinfo.siglistSz) },
     std::vector<arr::Vector<arr::zstring>> {
       { "data queue size", "signaling queue size" }, 
       { "value" }
     }
     );
  // organize as list of "connections", "buffering"
  return arr::make_cow<val::VList>
    (false, 
     vector<pair<string, val::Value>>{
       std::make_pair("connections"s, connections),
       std::make_pair("buffering"s, buffering)});
}


val::Value funcs::info_msg(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  // incoming requests ctx
  // incoming responses ctx
  auto info = ic.getMsgInfo();
  const std::vector<double> reqs(info.reqContexts.begin(), info.reqContexts.end());
  auto areqs = arr::make_cow<val::VArrayD>
    (false,
     arr::Vector<arr::idx_type>{reqs.size(),1}, 
     arr::Vector<double>(reqs.begin(), reqs.end()),
     std::vector<arr::Vector<arr::zstring>> {
       {}, 
       {"value"}
     }
     );
  const std::vector<Global::conn_id_t> rsps(info.rspContexts.begin(), info.rspContexts.end());
  auto arsps = arr::make_cow<val::VArrayD>
    (false,
     arr::Vector<arr::idx_type>{rsps.size(),1}, 
     arr::Vector<double>(rsps.begin(), rsps.end()),
     std::vector<arr::Vector<arr::zstring>> {
       {}, 
       {"value"}
     }
     );

  return arr::make_cow<val::VList>
    (false, 
     vector<pair<string, val::Value>>{
       std::make_pair("req contexts"s, areqs),
       std::make_pair("rsp contexts"s, arsps)});
}


val::Value funcs::info_ctx(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  // we potentially have:
  // - current state
  // - requests
  // - states
  const auto info = ic.getCtxInfo();
  return arr::make_cow<val::VArrayD>
    (false,
     arr::Vector<arr::idx_type>{3,1}, 
     arr::Vector<double>{
       static_cast<double>(info.state.reqid), 
       static_cast<double>(info.state.sourceid), 
       static_cast<double>(info.state.peerid)},
     std::vector<arr::Vector<arr::zstring>> {
       { "request id", "source id", "peer id" }, 
       {"value"}
     }
     );
}


val::Value funcs::str(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  std::cout << val::str(getVal(v[0]), cfg::cfgmap) << std::endl;
  ic.s->k->next->next->atype |= interp::Kont::SILENT;
  return val::VNull();
}


static val::Value cfgToVal(const cfg::CfgVariant& cv) {
  switch (cv.which()) {
  case cfg::INT:    return val::make_array(static_cast<double>(get<int64_t>(cv)));
  case cfg::DOUBLE: return val::make_array(static_cast<double>(get<double>(cv)));
  case cfg::STRING: return val::make_array(arr::zstring(get<std::string>(cv)));
  default:
    throw std::logic_error("cfgToVal(): unknown 'CfgVariant' type");
  }
}


static cfg::CfgVariant valToCfg(const val::Value& v) {
  switch (v.which()) {
  case val::vt_double: return cfg::CfgVariant(val::get_scalar<double>(v));
  case val::vt_string: return cfg::CfgVariant(string(val::get_scalar<arr::zstring>(v)));
  default:
    throw std::out_of_range("valToCfg(): invalid 'Value' type");
  }
}


val::Value funcs::_options(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  // works the same way as in R.
  if (v.empty()) {
    // if no args build a list from the config map and return:
    auto vl = arr::make_cow<val::VList>(false, vector<pair<string, val::Value>>{});
    for (const auto& e: cfg::cfgmap) {
      vl->push_back(std::make_pair(e.first, cfgToVal(e.second)));
    }
    return vl;
  }
  else {
    // else, for each arg, get and set, and return the sublist we've set:
    auto vl = arr::make_cow<val::VList>(false, vector<pair<string, val::Value>>{});
    for (const auto e: v) {
      try {
        cfg::cfgmap.set(getName(e), valToCfg(getVal(e)));
        vl->push_back(std::make_pair(getName(e), getVal(e)));
      }
      catch (std::out_of_range&) {
        throw interp::EvalException("unknown config variable '"s + getName(e) + "':", 
                                    getLoc(e));
      }
      catch (std::invalid_argument&) {
        throw interp::EvalException("invalid type for config variable '"s + getName(e) + "':", 
                                    getLoc(e));
      }
    }
    // because it would not be efficient to read from the map at each
    // log action, we make sure the log level reflects the
    // configuration (in case it was changed by the code above):
    lg.setLevel(zlog::from_string(get<std::string>(cfg::cfgmap.get("log.level"))));
    ic.s->k->next->next->atype |= interp::Kont::SILENT;
    return vl;
  }
}


val::Value funcs::make_connection(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum {IP, PORT};
  auto ip   = val::get_scalar<arr::zstring>(getVal(v[IP]));
  int  port = val::get_scalar<double>(getVal(v[PORT]));
  auto id = ic.connect(ip, port);
  return val::VConn(ip, port, id);
}


val::Value funcs::connection_port(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { CONN };
  auto connection = get<val::VConn>(getVal(v[CONN]));
  
  return val::make_array(static_cast<double>(connection.port));
}


val::Value funcs::connection_address(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { CONN };
  auto connection = get<val::VConn>(getVal(v[CONN]));
  
  return val::make_array(arr::zstring(connection.ip));
}


val::Value funcs::make_timer(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { DURATION, LOOP_MAX };
  const std::chrono::nanoseconds nanosecs = val::get_scalar<Global::duration>(getVal(v[DURATION]));
  const double loop_max = val::get_scalar<double>(getVal(v[LOOP_MAX]));
  auto loop = get<val::VCode>(ic.s->k->r->find("loop"));
  auto once = get<val::VCode>(ic.s->k->r->find("once"));
  auto timer = std::make_shared<val::VTimer>(nanosecs.count(), loop.expr, once.expr, loop_max);
  ic.addTimer(timer);
  return timer;
}


template<typename T, typename A1>
struct msync_wrapper {
  static val::Value f(const val::Value& val, A1 a1, 
                      const yy::location& loc_val, const yy::location& loc_a1) {
    const auto& v = get<T>(val);
    try {
      v->msync(a1);
    }
    catch (const std::exception& e) {
      throw interp::EvalException(e.what(), loc_val);
    }
    return val::VNull();
  }
};

val::Value funcs::msync(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { X, ASYNC };
  auto async = val::get_scalar<bool>(getVal(v[ASYNC]));
  ic.s->k->next->next->atype |= interp::Kont::SILENT;
  return apply_to_types2<msync_wrapper, 
                         bool,   // type of argument 1 for sort_wrapper::f()
                         val::vt_double, 
                         val::vt_bool, 
                         val::vt_time, 
                         val::vt_string, 
                         val::vt_duration, 
                         val::vt_zts,
                         val::vt_interval>(getVal(v[X]), async, 
                                           getLoc(v[X]), getLoc(v[ASYNC]));  
}

static std::string extract_string_elt(const val::Value& a, size_t idx) {
  switch (a.which()) {
  case val::vt_zts: {
    const auto z = get<val::SpZts>(a);
    return arr::convert<std::string, double>(z->getArray()[idx % z->getArray().size()]);
  }
  case val::vt_double: {
    const auto ai = get<val::SpVAD>(a);
    return arr::convert<std::string, double>((*ai)[idx % ai->size()]);
  }
  case val::vt_list: {
    const auto ai = get<val::SpVList>(a);
    return val::to_string((ai->a)[idx % ai->size()], cfg::cfgmap, true);
  }
  case val::vt_bool: {
    const auto ai = get<val::SpVAB>(a);
    return arr::convert<std::string, bool>((*ai)[idx % ai->size()]);
  }
  case val::vt_time: {
    const auto ai = get<val::SpVADT>(a);
    return arr::convert<std::string, Global::dtime>((*ai)[idx % ai->size()]);
  }
  case val::vt_duration: {
    const auto ai = get<val::SpVADUR>(a);
    return arr::convert<std::string, Global::duration>((*ai)[idx % ai->size()]);
  }
  case val::vt_interval: {
    const auto ai = get<val::SpVAIVL>(a);
    return arr::convert<std::string, tz::interval>((*ai)[idx % ai->size()]);
  }
  case val::vt_string: {
    const auto ai = get<val::SpVAS>(a);
    return arr::convert<std::string, std::string>((*ai)[idx % ai->size()]);
  }
  default:
    return val::to_string(a, cfg::cfgmap, true);
  }
}


val::Value funcs::paste(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  const std::string sep = (*get<val::SpVAS>(getVal(v[0])))[0];

  // all the others arguments are from the ellipsis:
  // get the max size of the elts:
  size_t maxlen = std::accumulate(v.begin()+2, v.end(), static_cast<size_t>(0),
                                    [](size_t a, const val::VBuiltinG::arg_t& b) {
                                      auto l = val::size(getVal(b));
                                      return l <= a ? a : l;
                                    });

  // in the case where the collapse argument is a string, we return a
  // collapsed vector, i.e. a string array of one element:
  if (getVal(v[1]).which() == val::vt_string) {
    const string collapse = (*get<val::SpVAS>(getVal(v[1])))[0];
    std::string s;
    for (size_t j=0; j<maxlen; ++j) {
      for (auto i=v.begin()+2; i!=v.end(); ++i) { 
        s += extract_string_elt(getVal(*i), j) + (i+1 != v.end() ? sep : "");
      }
      s += j < maxlen - 1 ? collapse : "";
    }
    return val::make_array<arr::zstring>(s);
  }
  // in the case where the collapse argument is NULL, we return a
  // vector of zstring:
  else {
    auto a = make_cow<val::VArrayS>(false, arr::idx_type{0});
    for (size_t j=0; j<maxlen; ++j) {
      std::string s;
      for (auto i=v.begin()+2; i!=v.end(); ++i) { 
        s += extract_string_elt(getVal(*i), j) + (i+1 != v.end() ? sep : "");
      }
      a->concat(s, "");
   }
    return a;
  }
}


val::Value funcs::cat(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { FILE, SEP, FILL, LABELS, APPEND };
  const auto& file   = val::get_scalar<arr::zstring>(getVal(v[FILE]));
  const auto& sep    = val::get_scalar<arr::zstring>(getVal(v[SEP]));
  const auto labelsflag = getVal(v[3]).which() == val::vt_string;
  const auto& labels = labelsflag ? get<val::SpVAS>(getVal(v[LABELS])) : 
    make_cow<val::VArrayS>(false, arr::idx_type{0});
  const auto& append = val::get_scalar<bool>(getVal(v[APPEND]));
  const int ellipsispos = 5;

  // fill can be double or bool; if it's bool and it's true, then we use width from config:
  size_t fill = getVal(v[FILL]).which() == val::vt_double ?
    static_cast<size_t>(val::get_scalar<double>(getVal(v[FILL]))) :
    (val::get_scalar<bool>(getVal(v[FILL])) ? static_cast<size_t>(get<int64_t>(cfg::cfgmap.get("width"s))) :
     std::numeric_limits<size_t>::max());
  
  std::ofstream of;
  if (file != "") {
    of.open(file, std::ofstream::out | (append ? std::ofstream::app : std::ofstream::trunc));
  }

  size_t labelidx = 0;
  size_t linesz = 0;                               
  bool isnewline = true;
  for (auto e=v.begin()+ellipsispos; e!=v.end(); ++e) {
    auto eltsz = val::size(getVal(*e)); 
    for (size_t j=0; j<eltsz; ++j) {
      const auto& es = extract_string_elt(getVal(*e), j);
      isnewline = linesz + es.size() + sep.size() > fill;
      if (isnewline) {
        if (labelsflag && labels->size()) {
          isnewline = true;
          ++labelidx;
          labelidx = labelidx % labels->size();
          (file == "" ? cout : of) << endl << (*labels)[labelidx] << " ";
          linesz = (*labels)[labelidx].size() + 1;
        }
        else {
          (file == "" ? cout : of) << endl;
          linesz = 0;
        }
      } 
      (file == "" ? cout : of) << es << sep;
      linesz += es.size() + sep.size();
      isnewline = false;
    }
  }
  (file == "" ? cout : of) << endl;
  ic.s->k->next->next->atype |= interp::Kont::SILENT;
  return val::VNull();
}
