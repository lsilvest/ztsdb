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


#include <iostream>
#include <vector>
#include <list>
#include <tuple>
#include "interp.hpp"
#include "interp_ctx.hpp"
#include "unop_binop_funcs.hpp"
#include "conversion_funcs.hpp"
#include "valuevar.hpp"
#include "interp_error.hpp"
#include "parser_ctx.hpp"
#include "anf.hpp"
#include "base_funcs.hpp"
#include "config.hpp"


// #define DEBUG

using namespace interp;

// implementation ------------


// note that we don't care about reentrancy here, all we need is that
// two calls from the same thread generate different identifiers:
static function<string(string)> gensym = GensymFun();


static bool isAtomic(const E* e) {
  switch (e->etype) {
  case etbool:  case etdouble:
  case etdtime:
  case etinterval:
  case etunop:
  case etbinop:
  case etsymbol:
  case etnull:
  case etstring:
  case etboundvar:
  case etfunction:
  case etinvoke:
    return true;
  default:
    return false;
  }
}


static val::Value evalAtom(const E* e,
                           const shared_ptr<BaseFrame> r,
                           zcore::InterpCtx& ic,
                           bool isFuncall=false)
{
#ifdef DEBUG
  cout << "evalAtom with e : " << to_string(*e) << endl;
  cout << "| control type :  " << et_to_string[e->etype] << endl;
  cout << "| env:            " << string(*r) << endl;
  cout << "| env p:          " << r << endl;
#endif
  switch (e->etype) {
  case etnull:
    return val::VNull();
  case etsymbol: {
    const auto s = static_cast<const Symbol*>(e);
    try {
      auto& val = r->findR(s->data, isFuncall); // will throw  
      if (val.which() == val::vt_future) {
        throw interp::FutureException(s->data);
      }
      return val::VPtr(val);
    }
    catch (interp::FutureException&) {
      throw;
    }
    catch (std::out_of_range& e) {
      throw interp::EvalException(e.what(), s->loc);
    }
  }
  case etboundvar: {
    const auto bv = static_cast<const Boundvar*>(e);
    val::Value val; 
    try {
      val = r->find(bv->data);        // will throw
    }
    catch (std::out_of_range& e) {
      throw interp::EvalException(e.what(), bv->loc);
    }
    if (val.which() == val::vt_future)
      throw interp::FutureException(bv->data);
    return val; 
  }
  case etdtime: {
    auto d = static_cast<const Dtime*>(e);
    return val::Value(d->data); }
  case etinterval: {
    auto d = static_cast<const Interval*>(e);
    return val::Value(d->data); }
  case etstring: {
    auto s = static_cast<const String*>(e);
    return val::Value(s->data); }
  case etunop: {
    auto u = static_cast<const Unop*>(e);
    const auto& e1 = evalAtom(u->e, r, ic);
    return funcs::evalunop(e1, int(u->op)); }
  case etbinop: {
    auto b = static_cast<const Binop*>(e);
    // choose integer for b.op LLL
    auto e1 = evalAtom(b->left, r, ic);
    const auto& e2 = evalAtom(b->right, r, ic);
    const auto& attrib = evalAtom(b->attrib, r, ic);
    resetRef(e1); // never pass by reference when using infix notation
    return funcs::evalbinop(e1, e2, int(b->op), attrib); }
  case etdouble: {
    auto f = static_cast<const Double*>(e);
    return f->data; }
  case etbool: {
    auto b = static_cast<const Bool*>(e);
    return val::Value(b->data); }
  case etfunction: {
    const auto f = static_cast<const Function*>(e);
    return val::Value(std::make_shared<val::VClos>(f)); }
  case etinvoke: {
    const auto inv = static_cast<const Invoke*>(e);
    val::Value bval;
    try {
      bval = r->up->findR(inv->data, true); // get it from up or we could get
                                            // the name of a named parameter
    }
    catch (std::out_of_range& e) {
      throw interp::EvalException(e.what(), inv->loc);
    }
    const auto& b = get<val::SpBuiltin>(bval);
    auto& bf = static_cast<BuiltinFrame&>(*r);
    if (bf.hasFutures()) {
      throw interp::FutureException("invoke");
    }
    b->checkArgs(bf);
    return (*b)(bf, ic);
  }
  default:
    throw interp::EvalException("unknown atomic expression: " + to_string(*e), e->loc);
  }
}


static int getUnused(int& idx, ElNode*& eln, const vector<bool>& uargs) {
  if (eln == nullptr) return -1;

  if (idx < 0) {
    idx = 0; 
  } 
  else {
    if (eln->next == nullptr) return -1;
    ++idx;
    eln = eln-> next;
  }

  while (uargs[idx]) {
    if (eln->next == nullptr) return -1;
    ++idx;
    eln = eln->next;
  }

  return idx;
} 



struct ArgInfo {
  E* name;                      // 0
  E* expr;                      // 1
  bool isFormal;                // 2
  bool isEllipsis;              // 3
  bool isRef;
};


/// Build vector or tuples <name, expression, formal, part of ellipis>
/// for all arguments. The resulting vector is an ordered list of
/// arguments to the function, in the order in which they were defined
/// (and matched) in the formlist.
static vector<ArgInfo> processArgs(int ellipsisPos,
                                   const map<string, int>& fargsMap,
                                   const El* fargs, /// formals args
                                   const El* aargs) /// actuals args
{
#ifdef DEBUG
  cout << "processArgs" << endl;
  cout << "| fargs(:      " << (fargs ? to_string(*fargs) : "") << ')' << endl;
  cout << "| aargs(:      " << (aargs ? to_string(*aargs) : "") << ')' << endl;
  cout << "| ellipsisPos: " << ellipsisPos << endl;
#endif

  // the following two are to track use (consumption) of the arguments as we go:
  auto ufargs = vector<bool>(fargs->n, false);
  auto ugargs = vector<bool>(aargs->n, false);

  // the result of the processing:
  auto res = vector<ArgInfo>(ellipsisPos >= 0 ? fargs->n - 1 : fargs->n);
  
  // 1. extract named actual args that match in formal args:
  auto geln = aargs->begin;
  for (unsigned n=0; n<aargs->n; ++n) {
    if (geln->e->etype == ettaggedexpr) { // tagged means named
      auto te = static_cast<TaggedExpr*>(geln->e);
      if (te->symb && te->symb->data != "") {
        auto idx = fargsMap.find(te->symb->data);
        if (idx != fargsMap.end()) {
          ugargs[n]           = true;
          if (ufargs[idx->second]) {
            throw interp::EvalException("formal argument \"" + 
                                        te->symb->data + "\" matched by multiple actual arguments",
                                        te->symb->loc);
          }
          ufargs[idx->second] = true;
          if (ellipsisPos >= 0 && idx->second > ellipsisPos) {
            // get proper index in case there is an ellipsis:
            res[idx->second-1] = ArgInfo{te->symb, te->e, false, false, te->symb->ref};
          } 
          else {
            res[idx->second] = ArgInfo{te->symb, te->e, false, false, te->symb->ref};
          }
        }
      }
    }
    geln = geln->next;
  }

  // 2. consume actual args by position matching in the formal args:
  geln = aargs->begin;
  auto feln = fargs->begin;
  int fidx = -1;
  int gidx = -1;
  fidx = getUnused(fidx, feln, ufargs); 
  gidx = getUnused(gidx, geln, ugargs);
  while (fidx >= 0 && gidx >= 0) {
    if (geln->e->etype == ettaggedexpr) {
      // if we find a tagged expression, it has to be part of an ellipsis
      if (ellipsisPos != -1) {
        auto te = static_cast<TaggedExpr*>(geln->e);
        res.emplace_back(ArgInfo{te->symb, te->e, false, true, te->symb->ref});
      }
      else {
        throw interp::EvalException("unused argument (" + to_string(*geln->e) + ')', geln->e->loc);
      }
    } else {
      assert(geln->e->etype == etarg);
      auto a = static_cast<Arg*>(geln->e);
      if (feln->e->etype == ettaggedexpr) {
        const auto te = static_cast<TaggedExpr*>(feln->e);
        res[fidx] = ArgInfo{te->symb, a->e, false, false, a->symb->ref};
        fidx = getUnused(fidx, feln, ufargs); 
      }
      else if (feln->e->etype != etellipsis) {
        res[fidx] = ArgInfo{feln->e, a->e, false, false, a->symb->ref};
        fidx = getUnused(fidx, feln, ufargs); 
      } 
      else {
        res.emplace_back(ArgInfo{a->symb, a->e, false, true, a->symb->ref});
      }
    }
    gidx = getUnused(gidx, geln, ugargs);
  }  
  
  // 3. if we have leftovers in the actuals, then it's an error, and we throw:
  if (gidx >= 0) {
    throw interp::EvalException("unused argument (" + to_string(*geln->e) + ')', geln->e->loc); 
  }

  // 4. if we have leftovers in ufargs, find the default for them and
  // populate 'res' accordingly; if we don't find, then throw:
  while (fidx >= 0) {
    if (feln->e->etype == ettaggedexpr) {
      auto te = static_cast<TaggedExpr*>(feln->e);
      if (ellipsisPos >= 0 && fidx > ellipsisPos) {
        // get proper index in case there is an ellipsis:
        res[fidx-1] = ArgInfo{te->symb, te->e, true, false, false};
      } 
      else {
        res[fidx] = ArgInfo{te->symb, te->e, true, false, false};
      }
      fidx = getUnused(fidx, feln, ufargs); 
    } 
    else if (feln->e->etype == etellipsis) {
      fidx = getUnused(fidx, feln, ufargs); 
    }
    else {
      if (geln) {
        throw interp::EvalException("argument missing: " + to_string(*feln->e), geln->e->loc);
      }
      else {
        throw std::range_error("argument missing: " + to_string(*feln->e));
      }
    }      
  }

  return res;
} 


static shared_ptr<Kont> applyProc(val::VClos& proc,
                                  shpfrm r, // the calling environment
                                  std::vector<shpfrm>& fstack,
                                  El* el, // the actual args
                                  shared_ptr<Kont>& k) {
#ifdef DEBUG
  cout << "applyProc " << val::to_string(proc) << endl;
  cout << "| with k : " << string(*k) << endl;
#endif

  fstack.push_back(std::make_shared<ClosureFrame>(r));
  auto fenv = fstack.back();
  if (fenv->getDepth() >= get<int64_t>(cfg::cfgmap.get("expressions"))) {
    throw EvalException("evaluation nested too deeply: infinite recursion / options(expressions=)?",
                        k->control ? k->control->loc : yy::missing_loc());
  }
  fenv->ec = r->ec;

  // put a sentinel continuation to make sure fenv is cleared:
  auto ksentinel = make_shared<Kont>(Kont{nullptr, nullptr, fenv, k, Kont::END});

  // don't lose time if the function takes no args:
  if (!proc.f->formlist || proc.f->formlist->n == 0) {
    if (el && el->n) {
      // we print only the first one (R prints all):
      throw interp::EvalException("unused argument (" + ::to_string(*el->begin->e) + ')', 
                                  el->begin->e->loc);
    }
    return make_shared<Kont>(Kont{nullptr, proc.f->body, fenv, ksentinel, Kont::NORMAL});
  } 
  // or if the function only has ellipsis and no args are given:
  else {
    if (proc.f->formlist->n == 1 && 
        proc.f->formlist->begin->e->etype == etellipsis && 
        (!el || el->n == 0)) {
      return make_shared<Kont>(Kont{nullptr, proc.f->body, fenv, ksentinel, Kont::NORMAL});
    }
  }

  auto paVec = processArgs(proc.ellipsisPos, proc.argMap, proc.f->formlist, el);
  auto kchain = make_shared<Kont>(Kont{nullptr, proc.f->body, fenv, ksentinel, Kont::NORMAL});

  for (auto i=static_cast<int>(paVec.size())-1; i>=0; --i) {
    auto er = paVec[i].isFormal ? fenv : r;
    auto atype = (paVec[i].isEllipsis ? Kont::ELLIPSIS : Kont::ARG) |
                 (paVec[i].isRef ? Kont::REF : 0);
    kchain = make_shared<Kont>(Kont{paVec[i].name, nullptr, fenv, kchain, atype}); 
    kchain = make_shared<Kont>(Kont{nullptr, paVec[i].expr, er, kchain, Kont::NORMAL}); 
  }

  return kchain;
}


static shared_ptr<Kont> applyBuiltin(const val::SpBuiltin& builtin,
                                     shpfrm r, // the calling environment
                                     std::vector<shpfrm>& fstack,
                                     const El* el,
                                     shared_ptr<Kont>& k) {
#ifdef DEBUG
  cout << "applyBuiltin " << to_string(builtin) << endl;
  cout << "| with k: " << string(*k) << endl;
#endif
  auto inv = builtin->invoke.get();
  Function* f = builtin->signature.get();
  fstack.push_back(std::make_shared<BuiltinFrame>(r, r->ec, builtin->argMap.size()));
  auto fenv = fstack.back();
#ifdef DEBUG
  cout << "|  created fenv:   " << fenv << endl;
#endif

  // put a sentinel continuation to make sure fenv is cleared:
  auto ksentinel = make_shared<Kont>(Kont{nullptr, nullptr, fenv, k, Kont::END});

  // don't lose time if the function takes no args:
  if (!f->formlist || f->formlist->n == 0) {
    if (el && el->n) {
      // we print only the first one (R prints all):
      throw interp::EvalException("unused argument (" + ::to_string(*el->begin->e) + ')',
                                  el->begin->e->loc);
    }
    return make_shared<Kont>(Kont{nullptr, inv, fenv, ksentinel, Kont::NORMAL});
  } 
  // or if the function only has ellipsis and no args are given:
  else {
    if (f->formlist->n == 1 && f->formlist->begin->e->etype == etellipsis && (!el || el->n == 0)) {
      return make_shared<Kont>(Kont{nullptr, inv, fenv, ksentinel, Kont::NORMAL});
    }
  }

  auto paVec = processArgs(builtin->ellipsisPos, builtin->argMap, f->formlist, el);
  auto kchain = make_shared<Kont>(Kont{nullptr, inv, fenv, ksentinel, Kont::NORMAL});


  for (auto i=static_cast<int>(paVec.size())-1; i>=0; --i) {
    const auto atype = (paVec[i].isEllipsis ? Kont::ELLIPSIS : Kont::ARG) |
                       (paVec[i].isRef ? Kont::REF : 0);
    const auto& name = static_cast<const Symbol*>(paVec[i].name);
    const auto& info = builtin->argInfo;
 
    bool noEval = atype & Kont::ARG ? 
      name && info.find(name->data) != info.end() && !info.at(name->data).doEval : 
      !builtin->evalEllipsis;
      
    if (noEval) {
      // when we have a reason we could add the ref information...
      if (atype == Kont::ARG) {
        assert(name);
        fenv->add(name->data, val::VCode(paVec[i].expr));
      }
      else {
        fenv->addEllipsis(name ? name->data : "",
                          val::VCode(paVec[i].expr),
                          paVec[i].expr->loc,
                          paVec[i].isRef);
      }
    } 
    else {
      auto er = paVec[i].isFormal ? fenv : r;
      if (paVec[i].name && paVec[i].expr) {
        paVec[i].name->loc = paVec[i].expr->loc; // we need to recover the
                                                 // location of the evaluated
      }                                          // argument and not of the symbol
      kchain = make_shared<Kont>(Kont{paVec[i].name, nullptr, fenv, kchain, atype});
      kchain = make_shared<Kont>(Kont{nullptr, paVec[i].expr, er, kchain, Kont::NORMAL});
    }
  }

  return kchain;
}


shared_ptr<Kont> interp::buildElChain(const ElNode* eln, 
                                      unsigned n, 
                                      shared_ptr<BaseFrame> r, 
                                      shared_ptr<Kont>& k) 
{
#ifdef DEBUG
  // cout << "buildElChain:" << endl;
  // cout << "| n:      " << n << endl;
  // cout << "| eln:    " << hex << eln << dec << endl;
  // if (eln->e) {
  //   cout << "| eln->e: " << to_string(*eln->e) << endl;
  // }
#endif
  auto nk = n == 1 ? k : buildElChain(eln->next, n-1, r, k);
  if (eln->e->etype == etleftassign) {
    auto la = static_cast<LeftAssign*>(eln->e);
    // next is not created here, protect the overwriting of an assignment:
    if (n == 1) {
      nk = make_shared<Kont>(Kont{nullptr, la->e1, r, nk, Kont::NORMAL});
      nk->next->atype |= Kont::SILENT;
    }
    auto rk = make_shared<Kont>(Kont{nullptr, la->e2, r, nk, Kont::NORMAL});
    rk->next->var = la->e1;
    return rk;
  } 
  else if (eln->e->etype == etspecialassign) {
    auto sa = static_cast<SpecialAssign*>(eln->e);
    // next is not created here, protect the overwriting of an assignment:
    if (n == 1) {
      nk = make_shared<Kont>(Kont{nullptr, sa->e1, r, nk, Kont::NORMAL});
      nk->next->atype |= Kont::SILENT;
    }
    auto rk = make_shared<Kont>(Kont{nullptr, sa->e2, r, nk, Kont::NORMAL});
    rk->next->var = sa->e1;
    rk->next->atype |= Kont::GLOBAL;
    return rk;
  }
  else {
    return make_shared<Kont>(Kont{nullptr, eln->e, r, nk, Kont::NORMAL});
  }
}


static shared_ptr<Kont> buildKont(const E* e, shared_ptr<BaseFrame> r, shared_ptr<Kont> k) 
{
  if (e->etype == etleftassign) {
    auto la = static_cast<const LeftAssign*>(e);
    k = make_shared<Kont>(Kont{nullptr, la->e1, r, k, Kont::NORMAL});
    k->next->atype |= Kont::SILENT;
    auto rk = make_shared<Kont>(Kont{nullptr, la->e2, r, k, Kont::NORMAL});
    rk->next->var = la->e1;
    return rk;
  } 
  else if (e->etype == etspecialassign) {
    auto sa = static_cast<const SpecialAssign*>(e);
    k = make_shared<Kont>(Kont{nullptr, sa->e1, r, k, Kont::NORMAL});
    k->next->atype |= Kont::SILENT;
    auto rk = make_shared<Kont>(Kont{nullptr, sa->e2, r, k, Kont::NORMAL});
    rk->next->var = sa->e1;
    rk->next->atype |= Kont::GLOBAL;
    return rk;
  }
  else {
    return make_shared<Kont>(Kont{nullptr, e, r, k, Kont::NORMAL});
  }
}


inline static shared_ptr<Kont> insertWhile(shared_ptr<Kont> k, std::vector<shpfrm>& fstack) {
#ifdef DEBUG
  cout << "insertWhile" << endl;
  cout << "| with k: " << string(*k) << endl;
#endif
  fstack.push_back(make_shared<ShadowFrame>(k->r->shared_from_this(), nullptr, k->r->ec, nullptr));
  auto r = fstack.back();

  auto w = static_cast<const While*>(k->control);

  static const Null null(yy::missing_loc()); // result of while is null
  k->next->atype |= interp::Kont::SILENT;    // but silence it
  auto nextk = make_shared<Kont>(Kont{
      k->next->var, 
      &null, 
      r, 
      k->next, 
      Kont::WHILE});
  
  shared_ptr<Kont> bodyk;
  if (w->e2->etype == etexprlist) {
    auto body = static_cast<const El*>(w->e2);
    bodyk = buildElChain(body->begin, body->n, r, nextk);      
  }
  else {
    bodyk = buildKont(w->e2, r, nextk);
  }
  bodyk->atype |= Kont::COND;

  shared_ptr<Kont> condk;
  if (w->e1->etype == etexprlist) {
    auto cond = static_cast<const El*>(w->e1);
    condk = buildElChain(cond->begin, cond->n, r, bodyk);
  }
  else {
    condk = buildKont(w->e1, r, bodyk);
  }

  r->bc = condk;
  r->cc = nextk;

  return condk;
}


static void setIfFuture(val::Value& val, shpfrm& frame) {
  if (val.which() == val::vt_future) {
    auto& future = get<val::SpFuture>(val);
    future->setvalptr(val, frame);
  }
}


inline static shared_ptr<Kont> applyKont(shared_ptr<Kont> k, 
                                         std::vector<shpfrm>& fstack, 
                                         val::Value&& val) 
{
#ifdef DEBUG
  cout << "apply cont" << endl;
  cout << "| k:        "  << string(*k) << endl;
  cout << "| val type: "  << val::vt_to_string.at(val.which()) << endl;
  cout << "| val:      "  << val::to_string(val) << endl;
#endif

  // REMEMBER: eval in current env but assign in next!

  // These sentinel continuations are used to correctly pop and clear
  // environments at the end of a function or builtin
  // evaluation. Clearing of an environment is crucial to avoid
  // circular references of shared_ptrs; these occur whenever a
  // function is defined, as a function references an environment that
  // references the function itself.
  //
  // Additionally, we need to keep closure environments and
  // environments that potentially contain code (e.g. tryCatch
  // alternative) alive for the duration of this function. For this we
  // push in a temporary stack 'tmpenv' the environments that are
  // (potentially) about to be deleted, so that they will be
  // (potentially) deleted only on exit of this function.
  std::vector<shpfrm> tmpenv;
  while (k->next->atype & Kont::END) {
    k->r->ec = k->r->bc = k->r->cc = nullptr;
    tmpenv.push_back(fstack.back());
    fstack.pop_back();
    k = k->next;
  }

  // -----------------------
  // in this first phase, we ensure the proper type of assignment is
  // made, i.e. local and global env, ellipsis and args:
  
  // get the assignment env:
  auto ar = k->next->r;
  if (k->next->atype & Kont::ELLIPSIS) {
    string sym = k->next->var ? static_cast<const Symbol*>(k->next->var)->data : "";
    const auto loc = k->control ? k->control->loc :
      (k->next->var ? k->next->var->loc : yy::missing_loc());
    auto& valref = ar->addEllipsis(sym, std::move(val), loc, k->next->atype & Kont::REF);
    setIfFuture(valref, ar);
  }
  else {

#ifdef DEBUG
    cout << "| assignment env p: " << hex << ar << dec << endl;
    cout << "| assignment env:   " << string(*ar) << endl;
    cout << "| global:           " << (k->next->atype & Kont::GLOBAL) << endl;
#endif 
    if (k->next->var) {
      if (k->next->var->etype == etsymbol) {
        string sym = static_cast<const Symbol*>(k->next->var)->data;
        const auto loc = k->control ? k->control->loc : k->next->var->loc;
        if (k->next->atype & Kont::ARG) {
          auto& valref = ar->addArg(sym, std::move(val), loc, k->next->atype & Kont::REF);
          setIfFuture(valref, ar);
        }
        else if (k->next->atype & Kont::GLOBAL) {
          auto& valref = ar->addSpecial(sym, std::move(val));
          setIfFuture(valref, ar);
        }
        else {
          if (sym[0] != '?') {
            if (isConst(val)) {
              throw std::range_error("cannot assign const reference object");
            }
            // if (!isTmp(val) && isLocked(val)) {
            //   throw std::range_error("cannot assign locked non-temporary object");   
            // }
            resetTmp(val);
          }
          auto& valref = ar->add(sym, std::move(val));
          setIfFuture(valref, ar);
          if (k->next->atype & Kont::REF) setRef(valref); else resetRef(valref);
        }
      }
      else {
        throw interp::EvalException("can't assign to non-symbol", k->next->var->loc);
      }
    }
    else {
      if (k->next->next) {     // figure out if we need to continue
        // As 'evalAtom' must be evaluated for side effect anyway,
        // assign it to '?tmp' which could be useful in a
        // debugger. NB: we do not set the val pointer in the future
        // here as '?tmp' will be overwritten; as the future pointer
        // is not set, the response will discarded, which is the
        // behaviour we want:
        // auto& valtmp = k->next->atype & Kont::ARG ?
        //   ar->addArg("?tmp", std::move(val), k->control ? k->control->loc : yy::missing_loc(), false) : 
        //   ar->add("?tmp", std::move(val));
        // setTmp(valtmp);

        // NB: the code above never worked properly; for some reason,
        // setting ?tmp still creates an additional (and unwanted of
        // course!) reference; we'll get back here when we need to
        // implement a debugger.        
      } else {
        // this is the last evaluation and, although it's without
        // assigment, the value might come as the result of a previous
        // assignment:
        isTmp(val) ? resetTmp(val) : setConst(val);
        auto& valref = ar->add(".Last.value", std::move(val));
        setLast(valref);
        setIfFuture(valref, ar);
        // if (k->next->atype & Kont::REF) setRef(valref); else resetRef(valref);//needed LLL?
      }
    }
  }

  // -----------------------
  // in this second phase we figure out the control flow, i.e. which
  // continuation we have to give to the program being interpreted:
  if (k->next->atype & Kont::COND) {
    if (funcs::isTrue(val)) {
      // if the condition is true, continue into the while:
      return k->next;
    } else {
      // else terminate the loop by going to the current continuation
      auto cc = k->next->r->cc;       
      k->next->r->clear();      // breaks the circular reference of
                                // shared ptrs (else bc will never be
                                // freed)
      return cc;
    }
  } 
  else if (k->next->atype & Kont::WHILE) {
    // at the end of the while body, go back to reexecute the condition:
    // cout << "begin continuation:" << string(*k->next->r->bc) << endl;      
   return k->next->r->bc;
  } 
  else {
    if (k->next->control) {
      //      cout << "k->next:" << string(*k->next) << endl;      
      return k->next;
    }
    else {
      //      cout << "k->next->next:"  << endl;      
      return k->next->next ? k->next->next : k->next;
    }
  }
}


shared_ptr<Kont> interp::step(shared_ptr<Kont>& k, vector<shpfrm>& fstack, zcore::InterpCtx& ic) {
#ifdef DEBUG
  cout << "step with control : " << to_string(*k->control) << endl;
  cout << "| cont            : " << string(*k) << endl;
  cout << "| env p           : " << k->r << endl;
  if (k->r) cout << "| env             : " << string(*k->r) << endl;
#endif

  // if atomic, we evaluate and apply the continuation
  if (isAtomic(k->control)) {
    return applyKont(k, fstack, evalAtom(k->control, k->r, ic));
  }
  else {
    switch (k->control->etype) {
    // Empty ----------- is not possible, as we take care of this in applyKont
    // LeftAssign ------
    case etleftassign:
    // SpecialAssign ---
    case etspecialassign:
      return buildKont(k->control, k->r, k->next);      
    // ExprList --------
    case etexprlist: {
      auto el = static_cast<const El*>(k->control);
      return buildElChain(el->begin, el->n, k->r, k->next);
    }
    case etcode: {              // e.g. lazy function args
      auto c = static_cast<const Code*>(k->control);
      return make_shared<Kont>(Kont{nullptr, c->e, k->r, k->next, Kont::NORMAL});
    }
    // Request ---------
    case etrequest: {
      // this is an outgoing request of the form 'con ? <expr>'
      // we don't block until the value is actually used
      auto req = static_cast<const Request*>(k->control);
      auto& atom = val::gval(evalAtom(req->e1, k->r, ic));
      auto& con = get<val::VConn>(atom); // throws bad get...
      // it should be a VCon, check that LLL
      // look up bound vars (::xyz)
      set<string> ss;
      getBoundVars(req->e2, ss);
      auto vl = make_cow<val::VList>(true, val::VList());
      for (auto i : ss) {
        vl->push_back(make_pair(i, k->r->find(i)));
      }
      // find the name, skipping over END continuations:
      auto kName = (k->next->atype & Kont::END) ? k->next : k;
      string name = kName->next->var ? 
        to_string(*static_cast<const Symbol*>(kName->next->var)) : "";
      auto future = std::make_shared<val::VFuture>();
      ic.sendReq(name, con, req->e2, vl, future);
      return applyKont(k, fstack, std::move(future));
    }
    // IfElse ---------
    case etifelse: {
      auto ie = static_cast<const IfElse*>(k->control);
      if (funcs::isTrue(val::gval(evalAtom(ie->e1, k->r, ic)))) {
        return make_shared<Kont>(Kont{nullptr, ie->e2, k->r, k->next, Kont::NORMAL});
      } else {
        return make_shared<Kont>(Kont{nullptr, ie->e3, k->r, k->next, Kont::NORMAL});
      }                         
    }
    // ExprSublist - function invocation -----
    case etfuncall: {
      auto es = static_cast<const Funcall*>(k->control);
      auto& proc = val::gval(evalAtom(es->e, k->r, ic, true)); // true: tell evalAtom we're looking for a funcall
      if (proc.which() == val::vt_clos) {
        return applyProc(*get<shared_ptr<val::VClos>>(proc), 
                         k->r->shared_from_this(), 
                         fstack, 
                         es->el, 
                         k->next);
      }
      else if (proc.which() == val::vt_builting) {
        return applyBuiltin(get<val::SpBuiltin>(proc), 
                            k->r->shared_from_this(), 
                            fstack, 
                            es->el, 
                            k->next);
      }
      else {
        throw interp::EvalException("cannot apply non-function", es->e->loc);
      }
    } 
    // While --------
    case etwhile:
      return insertWhile(k, fstack);
    // For --------
    case etfor: {
      auto forexpr = static_cast<const For*>(k->control);
      auto el = static_cast<const El*>(forexpr->forloop);
      k->next->atype |= interp::Kont::SILENT;
      return buildElChain(el->begin, el->n, k->r, k->next);
    }

    default:
      throw interp::EvalException("unknown expression: " + to_string(*k->control), k->control->loc);
    }
  }  
}
