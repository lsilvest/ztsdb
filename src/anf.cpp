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


#include "anf.hpp"
#include "misc.hpp"

// #define DEBUG


static E* norm(E*& e, El* el, ElNode*& eln, El* top, ElNode*& topn, bool doBndVar);
static void normTerm(E*& e, El* top, ElNode*& topn, bool doBndVar);
static E* strip(E*& e);


static bool isAtomic(const E* e) {	
  switch (e->etype) {
  case etbool:
  case etdouble:
  case etbinop:
  case etsymbol:
  case etboundvar:
  case etnull:
  case etnext:
  case etbreak:
  case etellipsis:
  case etstring:
  case etdtime:
  case etinterval:
  case ettaggedexpr:
    return true;
  default:
    return false;
  }
}


// note that we don't care about reentrancy here, all we need is that
// two calls from the same thread generate different identifiers:
static function<string(string)> gensym = GensymFun();


void anf::convertToANF(El* el, bool doBndVar) {
  ElNode* prev = nullptr;
  ElNode* cur  = el->begin;
  while (cur) {
    norm(cur->e, el, prev, el, prev, doBndVar);
    prev = cur;
    cur  = cur->next;
  }
}


static void normTerm(E*& e, El* top, ElNode*& topn, bool doBndVar) {
#ifdef DEBUG
  cout << "normTerm(" << to_string(*e) << ")" << endl;
  cout << "e->etype=" << et_to_string[e->etype] << endl;
#endif
  if (e->etype != etexprlist) {
    e = new El(e);
  }
  El* el = static_cast<El*>(e);
  ElNode* prev = nullptr;
  ElNode* cur  = el->begin;
  while (cur) {
    norm(cur->e, el, prev, top, topn, doBndVar);
    prev = cur;
    cur  = cur->next;
  }
  strip(e);
}


static inline void normName(E*& e, El* el, ElNode*& eln, El* top, ElNode*& topn, bool doBndVar) {
#ifdef DEBUG
  cout << "normName(" << to_string(*e) << ")" << endl;
  cout << "| e->etype=" << et_to_string[e->etype] << endl;
#endif
  strip(e);
  if (e->etype == etleftassign) {
    el->addafter(norm(e, el, eln, top, topn, doBndVar), eln);
    auto s  = gensym("?anf");
    auto la = static_cast<LeftAssign*>(e);
    auto newla = new LeftAssign(new Symbol(s, yy::missing_loc()), 
                                la->e1->clone(), 
                                yy::missing_loc());
    el->addafter(newla, eln);
    e = new Symbol(s, yy::missing_loc());  // clone it else we'll have double deletes    
  }
  else if (e->etype == etspecialassign) {
    auto la = static_cast<SpecialAssign*>(e);
    el->addafter(la, eln);
    e = la->e1->clone();  // clone it else we'll have double deletes    
  }
  else if (!isAtomic(e)) {
    auto s  = gensym("?anf");
    auto la = new LeftAssign(new Symbol(s, yy::missing_loc()), 
                             norm(e, el, eln, top, topn, doBndVar), 
                             yy::missing_loc());
    el->addafter(la, eln);
    e = new Symbol(s, yy::missing_loc());  // new it else we'll have double deletes
                        // during syntax tree deletion
  } else {
    e = norm(e, el, eln, top, topn, doBndVar);
  }
}


static inline void normEscape(E*& e, El* el, ElNode*& eln, El* top, ElNode*& topn) {
  normTerm(e, top, topn, true);
  auto s  = gensym("?bnd");
  auto newla = new LeftAssign(new Symbol(s, yy::missing_loc()), 
                              e, 
                              yy::missing_loc());
  top->addafter(newla, topn);
  e = new Boundvar(s, yy::missing_loc());  // clone it else we'll have double deletes    
}


/// For expression lists of one element, strip and delete the
/// expression list and return the element.
static E* strip(E*& e) {
  if (e->etype == etexprlist) {
    El* el = static_cast<El*>(e);
    if (el->n == 1) {
      e = el->begin->e;
      el->remove_nodelete(el->begin);
      delete el;
    }
  }
  return e;
}


static E* norm(E*& e, El* el, ElNode*& eln, El* top, ElNode*& topn, bool doBndVar) {
#ifdef DEBUG
  cout << "norm(" << to_string(*e) << ")" << endl;
  cout << "| e->etype=" << et_to_string[e->etype] << endl;
#endif
  
  switch (e->etype) {
  case etfuncall: {
    auto es = static_cast<Funcall*>(e);
    normName(es->e, el, eln, top, topn, doBndVar);
    if (es->el) {
      ElNode* elnfl = es->el->begin;
      for (unsigned n=0; n < es->el->n; ++n) {
        if (elnfl->e->etype == ettaggedexpr) {
          auto te = static_cast<TaggedExpr*>(elnfl->e);
          normTerm(te->e, top, topn, doBndVar);
        } else {
          normTerm(elnfl->e, top, topn, doBndVar);
        }
        elnfl = elnfl->next;
      }    
    }
    return es;
  }
  case etleftassign: {
    auto la = static_cast<LeftAssign*>(e);
    if (la->e2->etype == etfuncall) {
      norm(la->e2, el, eln, top, topn, doBndVar);
    }
    else {
      normName(la->e2, el, eln, top, topn, doBndVar);
    }
    return la;
  }
  case etspecialassign: {
    auto sa = static_cast<SpecialAssign*>(e);
    if (sa->e2->etype == etfuncall) {
      norm(sa->e2, el, eln, top, topn, doBndVar);
    }
    else {
      normName(sa->e2, el, eln, top, topn, doBndVar);
      return sa;
    }
  }
  case etunop: {
    auto u = static_cast<Unop*>(e);
    normName(u->e, el, eln, top, topn, doBndVar);
    return u;
  }
  case etbinop: {
    auto b = static_cast<Binop*>(e);
    normName(b->left, el, eln, top, topn, doBndVar);
    normName(b->right, el, eln, top, topn, doBndVar);
    normName(b->attrib, el, eln, top, topn, doBndVar);
    return b;
  }
  case etexprlist: {
    auto sel = static_cast<El*>(e);
    ElNode* elnfl = sel->begin;
    for (unsigned n=0; n < sel->n; ++n) {
      normTerm(elnfl->e, top, topn, doBndVar);
      elnfl = elnfl->next;
    }    
    return sel;
    //return normNameEl(sel, el, eln);
  }
  case etfunction: {
    auto f = static_cast<Function*>(e);
    // we normalize each element of the formlist separately:
    if (f->formlist) {
      ElNode* elnfl = f->formlist->begin;
      for (unsigned n=0; n < f->formlist->n; ++n) {
        // can be either ellipsis, symbol or tagged expression
        if (elnfl->e->etype == ettaggedexpr) {
          auto te = static_cast<TaggedExpr*>(elnfl->e);
          normTerm(te->e, top, topn, doBndVar);
          strip(te->e);
        }
        else if (!(elnfl->e->etype == etsymbol || elnfl->e->etype == etellipsis)) {
          throw domain_error("anf::norm: formlist has invalid type element");
        }
        elnfl = elnfl->next;
      }    
    }
    normTerm(f->body, top, topn, doBndVar);
    return f;
  }
  case etifelse: {
    auto i = static_cast<IfElse*>(e);
    normName(i->e1, el, eln, top, topn, doBndVar);
    normTerm(i->e2, top, topn, doBndVar);
    normTerm(i->e3, top, topn, doBndVar);
    return i;
  }
  case etwhile: {
    auto w = static_cast<While*>(e);
    normTerm(w->e1, top, topn, doBndVar);
    normTerm(w->e2, top, topn, doBndVar);
    return w;
  }
  case etfor: {
    auto f = static_cast<For*>(e);
    normTerm(f->forloop, top, topn, doBndVar);
    return f;
  }
  case etrequest: {
    auto r = static_cast<Request*>(e);
    normName(r->e1, el, eln, top, topn, doBndVar);
    normTerm(r->e2, el, eln, doBndVar);   // el, eln are the new top and topn
    return r;
  }    
  case etescape: {
    if (doBndVar) {
      auto esc = static_cast<Escape*>(e);
      normEscape(esc->e, el, eln, top, topn);
      e = esc->e;
      esc->e = nullptr;
      delete esc;
    }
    else {
      delete e;
      e = new Boundvar(gensym("?bnd"), yy::missing_loc());
    }
    return e;    
  }
  case etcode: {
    auto c = static_cast<Code*>(e);
    normName(c->e, el, eln, top, topn, doBndVar);
    return c;
  }
  default:
    return e;
  }
}

