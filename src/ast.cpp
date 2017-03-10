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
#include <stdexcept>
#include <sstream>
#include "ast.hpp"
#include "parser_utils.hpp"
#include "double.hpp"
#include "timezone/ztime.hpp"
#include "array.hpp"

extern tz::Zones tzones;

using namespace std;

const char* et_to_string[] = { 
  "empty",
  "null",
  "next",
  "break",
  "etellipsis",
  "bool",
  "double",
  "dtime",
  "interval",
  "symbol",
  "string",
  "boundvar",
  "escape",
  "invoke",
  "unop",
  "binop",
  "exprlist",
  "while",
  "ifelse",
  "for",
  "leftassign",
  "specialassign",
  "request",
  "taggedexpr",
  "function",
  "exprsublist",
  "code",
  "arg"
};


static function<string(string)> gensym = GensymFun();


For::For(Symbol* s, E* inexpr, E* body, loc_t l) : E(etfor, l) {
  // for (s in inexpr) is transormed like this:
  // ?foridx0 <- 1
  auto index = new Symbol(gensym("?foridx"), l);
  auto indexAssign = new LeftAssign(index, new Double(1, l), l);
  // ?inexpr0 <- inexpr
  auto inexprSymbol = new Symbol(gensym("?inexpr"), l);
  auto inexprAssign = new LeftAssign(inexprSymbol, inexpr, l);
  // length(inexpr)
  auto lengthExpr = new Funcall(new Symbol("length", l),
                                new El(new Arg(false, inexprSymbol->clone(), l)));
  // ?foridx0 < length(inexpr)
  auto whileCond = new Binop(yy::parser::token::LE, index->clone(), lengthExpr, l);
  
  // now create a body { s <- ?inexprSymbol[[?foridx0]]; body }
  // ?forIdx0 (the inside of the double assign)
  auto dlbAssignSublist = new El(new Arg(false, inexprSymbol->clone(), l));
  dlbAssignSublist->add(new Arg(false, index->clone(), l));
  //  ?inexprSymbol[[?foridx0]]
  auto dblsubset = new Funcall(new Symbol("dblsubset", l), dlbAssignSublist);
  // s <- ?inexprSymbol[[?foridx0]]
  auto symbolAssign = new LeftAssign(s, dblsubset, l);
  auto whileBody = new El(symbolAssign);
  whileBody->add(body);
  // and at the end of the body add the index increment (?foridx0 <- ?foridx0 + 1)
  // ?foridx0 + 1
  auto increment = new Binop(yy::parser::token::PLUS, index->clone(), new Double(1,l), l);
  auto incrementAssign = new LeftAssign(index->clone(), increment, l);
  whileBody->add(incrementAssign);

  // while (whileCond) whileBody
  auto fl = new El(new While(whileCond, whileBody, l));
  fl->addfirst(indexAssign);
  fl->addfirst(inexprAssign);
  forloop = fl;
}

// comma-separated:
string to_string_cs(const El& el) {
  stringstream ss;
  ElNode* eln = el.begin;
  for (unsigned n=0; n<el.n; ++n) {
    ss << to_string(*eln->e);
    if (n < el.n - 1) { ss << ", "; }
    eln = eln->next;
  }
  return ss.str();
}

string to_string(const E& e) {
  stringstream ss;
  switch (e.etype) {
  case etempty  : return "empty";
  case etnull   : return "NULL";
  case etbreak  : return "break";
  case etnext   : return "next";
  case etellipsis  : return "...";
  case etbool    : {
    auto edi = static_cast<const Bool*>(&e);
    return to_string((*edi->data)[0]);
  }
  case etdouble : {
    auto edi = static_cast<const Double*>(&e);
    const int digits = 6;
    return ztsdb::to_string((*edi->data)[0], digits);
  }
  case etdtime : {
    auto d = static_cast<const Dtime*>(&e);
    return tz::to_string((*d->data)[0], "", tzones.find("UTC"), "UTC");
  }
  case etinterval : {
    auto d = static_cast<const Interval*>(&e);
    return tz::to_string((*d->data)[0], "", tzones.find("UTC"), "UTC");
  }
  case etsymbol: {
    auto& s = static_cast<const Symbol&>(e);
    return s.ref ? "--" + s.data : s.data;
  }
  case etboundvar: {
    auto& s = static_cast<const Boundvar&>(e);
    return string("::") + s.data;
  }
  case etescape: {
    auto& s = static_cast<const Escape&>(e);
    return "++("s + to_string(*s.e) + ")"s;
  }
  case etinvoke: {
    auto& s = static_cast<const Invoke&>(e);
    return s.data;
  }
  case etstring: {
    auto& s = static_cast<const String&>(e);
    return (*s.data)[0];
  }  
  case etunop : {
    const Unop& u = static_cast<const Unop&>(e);
    return ztsdb::op_to_string.at(static_cast<const yy::parser::token::yytokentype>(u.op)) 
      + to_string(*u.e);
  }
  case etbinop : {
    const Binop& b = static_cast<const Binop&>(e);
    return "(" + to_string(*b.left)
      + ztsdb::op_to_string.at(static_cast<const yy::parser::token::yytokentype>(b.op))
      + to_string(*b.right) + ")";
  }
  case etexprlist : {
    auto& el = static_cast<const El&>(e);
    ElNode* eln = el.begin;
    if (el.n > 1) {
      ss << "{";
    }
    for (unsigned n=0; n<el.n; ++n) {
      ss << to_string(*eln->e);
      if (n < el.n-1) {
        ss << endl;
      }
      eln = eln->next;
    }
    if (el.n > 1) {
      ss << "}";
    }
    return ss.str();
  }
  case etwhile: {
    auto& w = static_cast<const While&>(e);
    return "while (" + to_string(*w.e1) + ") " + to_string(*w.e2);
  }
  case etifelse: {
    auto& i = static_cast<const IfElse&>(e);
    return "if (" + to_string(*i.e1) + ") " + 
      to_string(*i.e2) + " else " + to_string(*i.e3);
  }
  case etfor: {
    auto& f = static_cast<const For&>(e);
    return "for: {" + to_string(*f.forloop) + "}";
  }
  case etleftassign: {
    auto& la = static_cast<const LeftAssign&>(e);
    return to_string(*la.e1) + " <- " + to_string(*la.e2);
  }
  case etspecialassign: {
    auto& sa = static_cast<const SpecialAssign&>(e);
    return to_string(*sa.e1) + " <<- " + to_string(*sa.e2);
  }
  case etrequest: {
    auto& r = static_cast<const Request&>(e);
    return to_string(*r.e1) + " ? " + to_string(*r.e2);
  }  
  case ettaggedexpr: {
    auto& et = static_cast<const TaggedExpr&>(e);
    return to_string(*et.symb) + "=" + to_string(*et.e); 
  }
  case etarg: {
    auto& a = static_cast<const Arg&>(e);
    return (a.ref ? "ref: "s : ""s) + to_string(*a.e); 
  }
  case etfunction: {
    auto& f = static_cast<const Function&>(e);
    return "function(" 
      + (f.formlist ? to_string_cs(*f.formlist) : "") 
      + ") " 
      + to_string(*f.body);
  }
  case etfuncall: {
    auto& es = static_cast<const Funcall&>(e);
    return to_string(*es.e) + "(" + (es.el ? to_string_cs(*es.el) : "") + ")";
  }
  case etcode : {
    const auto& c = static_cast<const Code&>(e);
    return '<' + to_string(*c.e) + '>';
  }
  default:
    ss << "not implemented, type=" << e.etype;				// error LLL
    return ss.str();
  }
}


bool isEqual(const E* e1, const E* e2) {
  if (e1->etype != e2->etype) {
    return false;
  }

  switch (e1->etype) {
  case etempty   : 
  case etnull    : 
  case etbreak   : 
  case etnext    : 
  case etellipsis: return true;
  case etbool    : {
    auto b1 = static_cast<const Bool*>(e1);
    auto b2 = static_cast<const Bool*>(e2);
    return *b1->data == *b2->data;
  }
  case etdouble  : {
    auto d1 = static_cast<const Double*>(e1);
    auto d2 = static_cast<const Double*>(e2);
    return *d1->data == *d2->data;
  }
  case etdtime  : {
    auto d1 = static_cast<const Dtime*>(e1);
    auto d2 = static_cast<const Dtime*>(e2);
    return *d1->data == *d2->data;
  }
  case etsymbol  : {
    auto s1 = static_cast<const Symbol*>(e1);
    auto s2 = static_cast<const Symbol*>(e2);
    return s1->data == s2->data;
  }
  case etstring  : {
    auto s1 = static_cast<const String*>(e1);
    auto s2 = static_cast<const String*>(e2);
    return *s1->data == *s2->data;
  }
  case etinvoke  : {
    auto i1 = static_cast<const Invoke*>(e1);
    auto i2 = static_cast<const Invoke*>(e2);
    return i1->data == i2->data;
  }
  case etboundvar: { 
    auto b1 = static_cast<const Boundvar*>(e1);
    auto b2 = static_cast<const Boundvar*>(e2);
    return b1->data == b2->data;
  }   
  case etescape: { 
    auto esc1 = static_cast<const Escape*>(e1);
    auto esc2 = static_cast<const Escape*>(e2);
    return isEqual(esc1->e, esc2->e);
  }   
  case etunop : {
    auto u1 = static_cast<const Unop*>(e1);
    auto u2 = static_cast<const Unop*>(e2);
    return 
      u1->op == u2->op && 
      isEqual(u1->e, u2->e);
  }
  case etbinop : {
    auto b1 = static_cast<const Binop*>(e1);
    auto b2 = static_cast<const Binop*>(e2);
    return 
      b1->op == b2->op && 
      isEqual(b1->left, b2->left) && 
      isEqual(b1->right, b2->right);
  }
  case etexprlist : {
    auto& el1 = static_cast<const El&>(*e1);
    auto& el2 = static_cast<const El&>(*e2);
    if (el1.n != el2.n) {
      return false;
    }
    ElNode* eln1 = el1.begin;
    ElNode* eln2 = el2.begin;
    for (unsigned n=0; n<el1.n; ++n) {
      if (!isEqual(eln1->e, eln2->e)) {
        return false;
      }
      eln1 = eln1->next;
      eln2 = eln2->next;
    }
    return true;
    break;
  }
  case etwhile: {
    auto w1 = static_cast<const While*>(e1);
    auto w2 = static_cast<const While*>(e2);
    return 
      isEqual(w1->e1, w2->e1) && 
      isEqual(w1->e2, w2->e2);
    break;
  }
  case etifelse: {
    auto i1 = static_cast<const IfElse*>(e1);
    auto i2 = static_cast<const IfElse*>(e2);
    return 
      isEqual(i1->e1, i2->e1) && 
      isEqual(i1->e2, i2->e2) &&
      isEqual(i1->e3, i2->e3);
    break;
  }
  case etfor: {
    auto f1 = static_cast<const For*>(e1);
    auto f2 = static_cast<const For*>(e2);
    return isEqual(f1->forloop, f2->forloop);
   break;
  }
  case etleftassign: {
    auto a1 = static_cast<const LeftAssign*>(e1);
    auto a2 = static_cast<const LeftAssign*>(e2);
    return 
      isEqual(a1->e1, a2->e1) && 
      isEqual(a1->e2, a2->e2);
    break;
  }
  case etspecialassign: {
    auto a1 = static_cast<const SpecialAssign*>(e1);
    auto a2 = static_cast<const SpecialAssign*>(e2);
    return 
      isEqual(a1->e1, a2->e1) && 
      isEqual(a1->e2, a2->e2);
    break;
   }
  case etrequest: {
    auto r1 = static_cast<const Request*>(e1);
    auto r2 = static_cast<const Request*>(e2);
    return 
      isEqual(r1->e1, r2->e1) && 
      isEqual(r1->e2, r2->e2);
    break;
  }
  case ettaggedexpr: {
    auto te1 = static_cast<const TaggedExpr*>(e1);
    auto te2 = static_cast<const TaggedExpr*>(e2);
    return 
      isEqual(te1->symb, te2->symb) && 
      isEqual(te1->e,    te2->e);
    break;
  }
  case etarg: {
    auto a1 = static_cast<const Arg*>(e1);
    auto a2 = static_cast<const Arg*>(e2);
    return 
      a1->ref == a2->ref && 
      isEqual(a1->e,   a2->e);
    break;
  }
  case etfunction: {
    auto f1 = static_cast<const Function*>(e1);
    auto f2 = static_cast<const Function*>(e2);
    return 
      isEqual(f1->formlist, f2->formlist) && 
      isEqual(f1->body,     f2->body);
    break;
  }
  case etfuncall: {
    auto s1 = static_cast<const Funcall*>(e1);
    auto s2 = static_cast<const Funcall*>(e2);
    return 
      isEqual(s1->e,  s2->e) && 
      isEqual(s1->el, s2->el);
    break;
  }
  case etcode: {
    auto c1 = static_cast<const Code*>(e1);
    auto c2 = static_cast<const Code*>(e2);
    return isEqual(c1->e, c2->e);
    break;
  }
  default:
    throw std::domain_error(string("ast::isEqual: unknow etype ") + std::to_string(e1->etype));
  }
}

void getBoundVars(E* e, set<string>& ss) {
  switch (e->etype) {
  case etempty   : 
  case etnull    : 
  case etbreak   : 
  case etnext    : 
  case etellipsis: 
  case etbool    :
  case etdouble  : 
  case etdtime   :
  case etinterval:
  case etsymbol  :
  case etstring  : break;
  case etboundvar: { 
    auto& b = static_cast<const SElt<etboundvar>&>(*e);
    ss.insert(b.data);
    break;
  }  
  case etunop : {
    const Unop& u = static_cast<const Unop&>(*e);
    getBoundVars(u.e, ss);
    break;
  }
  case etbinop : {
    const Binop& b = static_cast<const Binop&>(*e);
    getBoundVars(b.left, ss);
    getBoundVars(b.right, ss);
    break;
  }
  case etexprlist : {
    auto& el = static_cast<const El&>(*e);
    ElNode* eln = el.begin;
    for (unsigned n=0; n<el.n; ++n) {
      getBoundVars(eln->e, ss);
      eln = eln->next;
    }
    break;
  }
  case etwhile: {
    auto& w = static_cast<const While&>(*e);
    getBoundVars(w.e1, ss);
    getBoundVars(w.e2, ss);
    break;
  }
  case etifelse: {
    auto& i = static_cast<const IfElse&>(*e);
    getBoundVars(i.e1, ss);
    getBoundVars(i.e2, ss);
    getBoundVars(i.e3, ss);
    break;
  }
  case etfor: {
    auto& f = static_cast<const For&>(*e);
    getBoundVars(f.forloop, ss);
    break;
  }
  case etleftassign: {
    auto& la = static_cast<const LeftAssign&>(*e);
    getBoundVars(la.e1, ss);
    getBoundVars(la.e2, ss);
    break;
  }
  case etspecialassign: {
    auto& sa = static_cast<const SpecialAssign&>(*e);
    getBoundVars(sa.e1, ss);
    getBoundVars(sa.e2, ss);
    break;
  }
  case etrequest: {
    auto& r = static_cast<const Request&>(*e);
    getBoundVars(r.e1, ss);
    // the following must not be run, because nested boundvars are to
    // be checked on the remote side of the request:
    // getBoundVars(r.e2, ss);
    break;
  }
  case ettaggedexpr: {
    auto& et = static_cast<const TaggedExpr&>(*e);
    getBoundVars(et.e, ss);
    break;
  }
  case etarg: {
    auto& a = static_cast<const Arg&>(*e);
    getBoundVars(a.e, ss);
    break;
  }
  case etfunction: {
    auto& f = static_cast<const Function&>(*e);
    getBoundVars(f.formlist, ss);
    getBoundVars(f.body, ss);
    break;
  }
  case etfuncall: {
    auto& es = static_cast<const Funcall&>(*e);
    getBoundVars(es.e, ss);
    getBoundVars(es.el, ss);
    break;
  }
  case etcode: {
    auto& c = static_cast<const Code&>(*e);
    getBoundVars(c.e, ss);
    break;
  }
  default:
    throw std::domain_error(string("ast::getBoundVars: unknown etype ") + std::to_string(e->etype));
  }
}

