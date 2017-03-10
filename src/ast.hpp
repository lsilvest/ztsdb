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


#ifndef AST_HPP
#define AST_HPP

#include <cstdint>
#include <iostream>
#include <cstring>
#include <vector>
#include <set>
#include <map>
#include "globals.hpp"
#include "timezone/interval.hpp"
#include "cow_ptr.hpp"
#include "string.hpp"
#include "array.hpp"
#include "main_parser/location.hpp"
#include "main_parser/position.hpp"


using namespace std;


enum Etype {
  etempty,
  etnull,
  etnext,
  etbreak,
  etellipsis,
  etbool,
  etdouble,
  etdtime,
  etinterval,
  etsymbol,
  etstring,
  etboundvar,
  etescape,
  etinvoke,
  etunop,
  etbinop,
  etexprlist,
  etwhile,
  etifelse,
  etfor,
  etleftassign,
  etspecialassign,
  etrequest,
  ettaggedexpr,
  etfunction,
  etfuncall,
  etcode,
  etarg,
};

using loc_t = yy::location;
using pos_t = yy::position;

extern const char* et_to_string[];

struct E {
  E(Etype et, const loc_t& l) : etype(et), loc(l) { 
    // std::cout << "E loc: " << l << std::endl;
  }
  virtual E* clone() const { return new E(*this); }
  virtual bool operator==(const E& o) const { return etype == o.etype; }
  Etype etype;
  virtual ~E() { }
  loc_t loc;
  
private:
  E(const E& e) : etype(e.etype), loc(e.loc) { } 
};

template<Etype t> 
struct Et : E {
  Et(const loc_t& l) : E(t, l) { }
  virtual Et* clone() const { return new Et(*this); }
  
private:
  Et(const Et& e) : E(t, e.loc) { }
};

using Null     = Et<etnull>;
using Next     = Et<etnext>;
using Break    = Et<etbreak>;
using Ellipsis = Et<etellipsis>;


template<Etype t> 
struct SElt : E {
  SElt(const string& s, const loc_t& l) : E(t, l), data(s) { }
  string data;
  virtual SElt* clone() const { return new SElt(*this); }
  virtual bool operator==(const SElt& s) const { 
    return data == s.data; 
  }
private:
  SElt(const SElt& e) : E(t, e.loc), data(e.data) { }
};

using Invoke     = SElt<etinvoke>;
using Boundvar   = SElt<etboundvar>;


struct Symbol : E {
  Symbol(const string& s, const loc_t& l, bool ref_p=false) : 
    E(etsymbol, l), data(s), ref(ref_p) { }

  string data;
  const bool ref;               // is it a reference?

  virtual Symbol* clone() const { return new Symbol(*this); }
  virtual bool operator==(const Symbol& s) const { return data == s.data; }
  ~Symbol() { }
private:
  Symbol(const Symbol& e) : E(etsymbol, e.loc), data(e.data), ref(e.ref)  { }
};


struct ElNode {
  ElNode(E* e_p) : e(e_p), next(nullptr) { }
  E* e;
  ElNode* next;
  ~ElNode() { 
    if (e) delete e;
  }
};

struct El : E {			// Expression list element
  El() : E(etexprlist, yy::missing_loc()), n(0), begin(nullptr), end(nullptr)  { }
  El(E* e) : E(etexprlist, e->loc) {
    begin = end = new ElNode(e);
    n = 1;
  }

  // update locations as expressions are added! LLL
  El* add(E* e) {
    ElNode* eln = new ElNode(e);
    if (end) {
      end->next = eln;
    } else {
      begin = eln;
    }
    end = eln;
    ++n;
    return this;
  }
  
  El* addfirst(E* e) {
    ElNode* eln = new ElNode(e); 
    eln->next = begin;
    begin = eln;
    if (!end) {
      end = eln;
    }
    ++n;
    return this;
  }

  El* addafter(E* e, ElNode*& here) {
    if (here) {
      ElNode* eln = new ElNode(e); 
      eln->next = here->next;
      here->next = eln;
      if (end == here) {
        end = eln;
      }
      here = eln;               // so multiple insertions are sequential
      ++n;
    }
    else {
      addfirst(e);
      here = begin;
    }
    return this;
  }

  /// Remove the node pointed to by 'here', but don't delete the expression.
  El* remove_nodelete(ElNode*& here) {
    auto node = here;
    here = here->next;
    node->e = nullptr;
    node->next = nullptr;
    delete node;
    --n;
    return this;
  }

  virtual El* clone() const { return new El(*this); }
  virtual bool operator==(const El& el) const { 
    if (n != el.n) {
      return false;
    }
    
    bool equal = true;
    ElNode* iter  = begin;
    ElNode* iters = el.begin;
    for (unsigned i=0; i<n; ++i) {
      if (!(*iter->e == *iters->e)) {
	return false;
      } else {
	iter = iter->next;
	iters = iters->next;
      }
    }
    return equal;
  }

  unsigned n;			// number of elements
  ElNode* begin;
  ElNode* end;
  ~El() {
    ElNode* iter  = begin;
    ElNode* next  = nullptr;
    for (unsigned i=0; i<n; ++i) {
      next = iter->next;
      delete iter;
      iter = next;
    }
  }
  
private:
  El(const El& el) : E(etexprlist, el.loc) {
    n = el.n;
    if (el.begin) {
      begin = new ElNode(el.begin->e->clone());
      ElNode* iterclone = begin;
      ElNode* iter = el.begin;
      while (iter->next) {
        iterclone->next = new ElNode(iter->next->e->clone());
        iterclone = iterclone->next;
        iter = iter->next;
      }
      end = iterclone; 
    }
    else {
      begin = el.begin;
      end = el.end;
    }
  }
};

template<Etype t> 
struct EEl : E {
  EEl(E* e_p, El* el_p) : 
    E(t, yy::span_loc(e_p->loc, el_p->loc)), e(e_p), el(el_p) { }
  EEl(E* e_p) : E(t, e_p->loc), e(e_p), el(nullptr) { }
  virtual EEl* clone() const { return new EEl(*this); }
  virtual bool operator==(const EEl<t>& eel) const { 
    return *e == *eel.e ? *el == *eel.el : false;
  }

  E* e;
  El* el;
  ~EEl() { delete e; delete el; }

private:
  EEl(const EEl& ee) : E(t, ee.loc), e(ee.e->clone()), el(ee.el->clone()) { }
};

using Funcall  = EEl<etfuncall>;

struct Binop : E {
  Binop(unsigned op_p, E* l, E* r, loc_t loc_p, E* a=nullptr) : 
    E(etbinop, loc_p), op(op_p), left(l), right(r), attrib(a ? a : new Null(yy::missing_loc())) { }
  virtual Binop* clone() const { return new Binop(*this); }
  virtual bool operator==(const Binop& b) const { 
    return op == b.op ? 
      *left == *b.left ? 
      *right == *b.right ? 
      *attrib == *b.attrib : false : false : false;
  }

  unsigned op;
  E* left;
  E* right;
  E* attrib;
  ~Binop() { delete left; delete right; delete attrib; }

private:
  Binop(const Binop& b) : 
    E(etbinop, b.loc), 
    op(b.op), 
    left(b.left->clone()), 
    right(b.right->clone()),
    attrib(b.attrib->clone()) { }
};

struct Unop : E {
  Unop(unsigned op_p, E* e_p, loc_t l) : E(etunop, l), op(op_p), e(e_p) { }
  virtual Unop* clone() const { return new Unop(*this); }
  virtual bool operator==(const Unop& u) const { 
    return op == u.op ? (*e == *u.e) : false;
  }

  unsigned op;
  E* e;
  ~Unop() { delete e; }

private:
  Unop(const Unop& u) : E(etunop, u.loc), op(u.op), e(u.e->clone()) { }
};


template<Etype t> 
struct E1 : E {
  E1(E* e_p, loc_t l) : E(t, l), e(e_p) { }
  virtual E* clone() const { return new E1(*this); }
  virtual bool operator==(const E1<t>& ee) const { 
    return *e == *ee.e;
  }

  E* e;
  ~E1() { if (e) delete e; }

private:
  E1(const E1& e) : E(t, e.loc), e(e.e->clone()) { }
};

using Code   = E1<etcode>;
using Escape = E1<etescape>;


template<Etype t> 
struct E2 : E {
  E2(E* e1_p, E* e2_p, loc_t l) : E(t, l), e1(e1_p), e2(e2_p) { }
  virtual E2* clone() const { return new E2(*this); }
  virtual bool operator==(const E2<t>& e) const { 
    return *e1 == *e.e1 ? (*e2 == *e.e2) : false;
  }

  E* e1;
  E* e2;
  ~E2() { delete e1; delete e2; }

private:
  E2(const E2& e) : E(t, e.loc), e1(e.e1->clone()), e2(e.e2->clone()) { }
};

using While = E2<etwhile>;
using LeftAssign = E2<etleftassign>;
using SpecialAssign = E2<etspecialassign>;
using Request = E2<etrequest>;  // 'e1' is the connection and 'e2' the request itself

template<Etype t> 
struct E3 : E {			// ifelse, while
  E3(E* e1_p, E* e2_p, E* e3_p, loc_t l) : E(t, l), e1(e1_p), e2(e2_p), e3(e3_p) { }
  virtual E3* clone() const { return new E3(*this); }
  virtual bool operator==(const E3<t>& e) const { 
    return *e1 == *e.e1 ? 
      (*e2 == *e.e2 ? 
       (*e3 == *e.e3) : false) : false;
  }

  E* e1;
  E* e2;
  E* e3;
  ~E3() { delete e1; delete e2; delete e3; }

private:
  E3(const E3& e) : E(t, e.loc), e1(e.e1->clone()), e2(e.e2->clone()), e3(e.e3->clone()) { }
};

using IfElse = E3<etifelse>;

struct For : E {
  For(Symbol* s, E* e1, E* e2, loc_t l);
  For(E* forloop_p, loc_t l) : 
    E(etfor, l), forloop(forloop_p) { } // only used in decoding, and
                                        // please note that forloop_p
                                        // relinquishes ownership to
                                        // forloop!
  virtual For* clone() const { return new For(*this); }
  virtual bool operator==(const For& f) const { return *forloop == *f.forloop; }

  E* forloop;
  ~For() { delete forloop; }

private:
  For(const For& f) : E(etfor, f.loc), forloop(f.forloop->clone()) { }
};

template<Etype t, typename T>
struct EData : E {		// double, int, bool
  typedef arr::cow_ptr<arr::Array<T>> ptr;
  typedef T value_type;

  EData(T d, loc_t l) : 
    E(t, l), data(arr::make_cow<arr::Array<T>>(false, 
                                               arr::Vector<arr::idx_type>{1}, 
                                               arr::Vector<T>{d}, 
                                               vector<arr::Vector<arr::zstring>>())) { }
  virtual EData* clone() const { return new EData(*this); }
  virtual bool operator==(const EData& ed) const { 
    return *data == *ed.data;
  }

  arr::cow_ptr<arr::Array<T>> data;
private:
  EData(const EData& e) : E(t, e.loc), data(e.data) { }
};

using Bool=EData<etbool, bool>;
using Double=EData<etdouble, double>;
using Interval=EData<etinterval, tz::interval>;
using Dtime=EData<etdtime, Global::dtime>;
using String=EData<etstring, arr::zstring>;

struct TaggedExpr : E {
  TaggedExpr(Symbol* s, E* e_p, loc_t l) : E(ettaggedexpr, l), symb(s), e(e_p) { }
  virtual TaggedExpr* clone() const { return new TaggedExpr(*this); }
  virtual bool operator==(const TaggedExpr& te) const { 
    return *symb == *te.symb ? (*e == *te.e) : false;
  }

  Symbol* symb;                 // a string would do? LLL
  E* e;
  ~TaggedExpr() { delete symb; delete e; }

private:
  TaggedExpr(const TaggedExpr& te) :
    E(ettaggedexpr, te.loc), symb(te.symb->clone()), e(te.e->clone()) { }
};


struct Arg : E {
  Arg(bool ref_p, E* e_p, loc_t l) : E(etarg, l), ref(ref_p), e(e_p) { }
  virtual Arg* clone() const { return new Arg(*this); }
  virtual bool operator==(const Arg& a) const { 
    return ref == a.ref ? (*e == *a.e) : false;
  }

  bool ref;
  E* e;
  ~Arg() { delete e; }

private:
  Arg(const Arg& a) :
    E(etarg, a.loc), ref(a.ref), e(a.e->clone()) { }
};


struct Function : E {
  Function(E* b, loc_t l) : E(etfunction, l), formlist(nullptr), body(b) { }
  Function(El* fl, E* b, loc_t l) : E(etfunction, l), formlist(fl), body(b) { }

  virtual Function* clone() const { return new Function(*this); }
  virtual bool operator==(const Function& f) const { 
    return *formlist == *f.formlist ? *body == *f.body : false;
  }

  El* formlist;
  E* body;

  void processFormlist(map<string, int>& argMap, int& ellipsisPos) {
    ellipsisPos = -1;
    if (formlist) {
      ElNode* eln = formlist->begin;
      for (unsigned n=0; n<formlist->n; ++n) {
        if (eln->e->etype == ettaggedexpr) {
          auto te = static_cast<TaggedExpr*>(eln->e);
          auto res = argMap.emplace(te->symb->data, static_cast<int>(n));
          if (!res.second) {
            throw range_error("repeated formal argument '" + te->symb->data + "'");
          }
        } 
        else if (eln->e->etype == etsymbol) {
          auto symb = static_cast<Symbol*>(eln->e);
          auto res = argMap.emplace(symb->data, static_cast<int>(n));
          if (!res.second) {
            throw range_error("repeated formal argument '" + symb->data + "'");
          }
        } 
        else {
          if (ellipsisPos != -1) {
            throw std::range_error("repeated formal argument '...'");
          }
          else {
            ellipsisPos = static_cast<int>(n);
          }
        }
        eln = eln->next;
      }
    }
  }

  ~Function() { if (formlist) delete formlist; delete body; }
  
private:
  Function(const Function& f) : 
    E(etfunction, f.loc), 
    formlist(f.formlist ? f.formlist->clone() : nullptr), 
    body(f.body->clone()) { }
};



string to_string(const E& e);
// comma-separated:
string to_string_cs(const El& el);

void getBoundVars(E* e, set<string>& ss);

bool isEqual(const E*, const E*);


#endif
