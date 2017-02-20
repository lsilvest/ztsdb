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


#include "zconnection.hpp"
#include "zconversion.hpp"

#include <sys/eventfd.h>
#include <ztsdb/parser_ctx.hpp>
#include <ztsdb/load_builtin.hpp>
#include <ztsdb/anf.hpp>
#include <ztsdb/globals.hpp>
#include <ztsdb/logging.hpp>


// #define _DEBUG

tz::Zones tzones("/usr/share/zoneinfo"); // should be configurable LLL
zlog::Logger lg;
cfg::CfgMap cfg::cfgmap;


static interp::shpfrm base    = make_shared<interp::Frame>("base"s);
static interp::shpfrm global  = make_shared<interp::Frame>("global"s,  global, base);


static void* executeNetHandler(void* args_p) {
  auto args = static_cast<std::pair<net::NetHandler*, volatile bool&>*>(args_p);
  auto c = args->first;
  try {
    c->run(args->second);
  }
  catch (std::exception& e) {
    std::cout << "exception is: " << e.what() << std::endl;
    lg.log(zlog::SV_ERROR, e.what());
    throw;
  }

  return nullptr;
}


Zconnection::Zconnection(std::string _addr, int _port)  : addr(_addr), port(_port) {
  if (!com) {
    int data_com_ir = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if (data_com_ir == -1) {
      throw std::system_error(std::error_code(errno, std::system_category()), 
                              "eventfd failed for data_com_ir");
    }
    int sig_com_ir = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if (sig_com_ir == -1) {
      throw std::system_error(std::error_code(errno, std::system_category()), 
                              "eventfd failed for sig_com_ir");
    }

    com = std::make_unique<net::NetHandler>("", 0, data_com_ir, sig_com_ir);

    // it's ugly this needs to be static... has to be a better way LLL
    static auto args = std::pair<net::NetHandler*, volatile bool&>{com.get(), stop};
    pthread_t comThread;
    pthread_create(&comThread, NULL, executeNetHandler, &args);

    net = std::make_unique<client::Client>(*com, data_com_ir, sig_com_ir);
  } // end if (!com)

  while (!com->ready); // wait until com is ready for connection requests

  peer_conn_id = net->connect(_addr, port);
  // check if no error, LLL
}


Zconnection::~Zconnection() {
  // want to stop the threads and exit cleanly
  // LLL
}


template <int RTYPE>
static std::vector<arr::Vector<arr::zstring>> rnamesToArrNames(const Rcpp::Vector<RTYPE>& v) {
  SEXP snames = Rf_isNull(v.attr("dim")) ? v.attr("names") : v.attr("dimnames");
  if (Rf_isNull(snames)) {
    return std::vector<arr::Vector<arr::zstring>>();
  }
  else {
    std::vector<arr::Vector<arr::zstring>> dimnames;
    // 'Rcpp::List::create' will put the nsnames as its first element
    // and not tranform it into a list like 'Rcpp::List::List' does:
    Rcpp::List rnames = Rf_isNull(v.attr("names")) ? Rcpp::List(snames) : Rcpp::List::create(snames);
    for (size_t i=0; i<static_cast<size_t>(rnames.size()); ++i) {
      auto svecnames = rnames[i];
      if (!Rf_isNull(svecnames)) {
        Rcpp::CharacterVector cvecnames(svecnames);
        arr::Vector<arr::zstring> vzs;
        for (auto n : cvecnames) {
          vzs.push_back(std::string(n));
        }
        dimnames.push_back(vzs);
      }
      else {
        dimnames.push_back(arr::Vector<arr::zstring>());
      }
    }
    return dimnames;
  }
}


template <int RTYPE>
static arr::Vector<arr::idx_type> rdimsToArrdims(const Rcpp::Vector<RTYPE>& v) {
  SEXP sdim = v.attr("dim");
  if (!Rf_isNull(sdim)) {
    Rcpp::IntegerVector idim(sdim);
    return arr::Vector<arr::idx_type>(idim.begin(), idim.end());
  }
  else {
    return arr::Vector<arr::idx_type>{static_cast<arr::idx_type>(v.size())};
  }
}


template <int RTYPE, typename T>
static arr::Array<T> convertRVector(const Rcpp::Vector<RTYPE>& v) {
  return arr::Array<T>(rdimsToArrdims(v),
                       arr::Vector<T>(v.begin(), v.end()),
                       rnamesToArrNames(v));
}

template <>
arr::Array<arr::zstring> convertRVector<STRSXP, arr::zstring>(const Rcpp::CharacterVector& v) {
  arr::Vector<arr::zstring> av;
  for (auto e: v) {
    av.push_back(CHAR(e));
  }
  return arr::Array<arr::zstring>(rdimsToArrdims(v), av, rnamesToArrNames(v));
}


static arr::Vector<Global::dtime> convertToDtime(const Rcpp::NumericVector& v) {
  arr::Vector<Global::dtime> av;
  for (auto e: v) {
    auto intPart = static_cast<ssize_t>(e);
    auto fracPart = static_cast<ssize_t>((e - intPart) * 1e9);
    av.push_back(Global::dtime(intPart*1s + fracPart*1ns));
  }
  return av;
}


template <>
arr::Array<Global::dtime> convertRVector<REALSXP, Global::dtime>(const Rcpp::NumericVector& v) {
  return arr::Array<Global::dtime>(rdimsToArrdims(v), convertToDtime(v), rnamesToArrNames(v));
}


template <int RTYPE, typename T>
static val::Value SEXPToArr(SEXP s) {
  Rcpp::Vector<RTYPE> v = Rcpp::as<Rcpp::Vector<RTYPE>>(s); // 'as' does a copy...
  return arr::make_cow<arr::Array<T>>(false, convertRVector<RTYPE, T>(v));
}


static val::Value SEXPToZts(const Rcpp::NumericVector& s) {
  if (CHAR(Rcpp::CharacterVector(s.attr("tclass"))[0]) != "POSIXct"s) {
    Rcpp::stop("can't handle non-POSIXct index in xts");
  }
  auto idx = Rcpp::NumericVector(s.attr("index"));
  return arr::make_cow<arr::zts>(false, 
                                 convertToDtime(idx), 
                                 convertRVector<REALSXP, double>(s));
}


static val::Value SEXPToValue(SEXP s) {
  switch(TYPEOF(s)){
  case REALSXP: {
    auto nv = Rcpp::NumericVector(s);
    if (!Rf_isNull(nv.attr("class")) && 
        CHAR(Rcpp::CharacterVector(nv.attr("class"))[0]) == "POSIXct"s) {
      return SEXPToArr<REALSXP, Global::dtime>(s);
    }
    else if (!Rf_isNull(nv.attr("class")) && 
             CHAR(Rcpp::CharacterVector(nv.attr("class"))[0]) == "xts"s) {
      return SEXPToZts(nv);      
    }
    else {
      return SEXPToArr<REALSXP, double>(s);
    }
  }
  case INTSXP: 
    return SEXPToArr<REALSXP, double>(s);
  case LGLSXP: 
    return SEXPToArr<LGLSXP, bool>(s);
  case STRSXP: 
    return SEXPToArr<STRSXP, arr::zstring>(s);
  default:
    Rcpp::stop("only bound variables of type numeric, logical and string are supported");
  }
}


/// Given a SEXP, check if this is of the form ++(expression). Return
/// the expression if this is the case, 'R_NilValue' otherwise.
static SEXP checkEscape(SEXP e) {
  if (!Rf_isNull(e) && TYPEOF(e) == LANGSXP) {
    auto symb = CAR(e);
    if (std::string(CHAR(PRINTNAME(symb))) == "+") {
      auto l = CDR(e);
      if (TYPEOF(l) == LISTSXP) {
        auto next = CAR(l);
        if (TYPEOF(next) == LANGSXP && std::string(CHAR(PRINTNAME(CAR(next)))) == "+") {
          return CAR(CDR(next));
        }
      }
    }
  }
  return R_NilValue;
}


#ifdef _DEBUG
// see below the comment about calling R_inspect
extern "C" SEXP R_inspect(SEXP x);
#endif


/// Finds an escape sequence. And escape sequence is started by a
/// '++'. So for example if we have 'con ? 1 - ++a', the variable 'a'
/// will be evaluated in R before the expression is being sent for
/// remote evaluation. If we have 'con ? 1 - ++(sin(2.2) + 4)', the
/// whole '++(sin(2.2) + 4)' will be evaluated in R before being sent
/// as part of the query.
static void findEscape(SEXP e, std::vector<SEXP>& ve) {
  // 'R_inspect' prints out the detailed AST. But in order to be able
  // to call it we need to recompile R without the 'attribute_hidden'
  // specifier in front of the definition of 'R_inspect'. This
  // specifier prevents visibility in the shared library and it
  // defined like this:
  //
  // # define attribute_hidden __attribute__ ((visibility ("hidden")))
#ifdef _DEBUG
  Rcpp::print(R_inspect(e));
#endif
  switch(TYPEOF(e)) {
  case LANGSXP: {
    while (e != R_NilValue) {
      auto res = checkEscape(e);
      if (res != R_NilValue) {
        ve.push_back(res);
      }
      findEscape(CAR(e), ve);
      e = CDR(e);
    }
    break;
  }
  default:
    ;
  }
}


SEXP Zconnection::query(std::string s, SEXP e, SEXP env) {
  ParserCtx pctx;
#ifdef _DEBUG
  //pctx.trace_scanning = true;
  //pctx.trace_parsing  = true;
  std::cout << "Zconnection::query: " << std::endl;
  std::cout << "| type of e: " << TYPEOF(e) << std::endl;
  Rcpp::print(e);
#endif

  auto res = pctx.parse(std::make_shared<std::string>(s + "\n"s));
  if (res != 0) {
    Rcpp::stop("error parsing"s + s);
  }
  
#ifdef _DEBUG
  std::cout << to_string(*pctx.prog.get()) << std::endl;
#endif
  // 'convertRVector' has a second argument which when set to false
  // allows for a substitution of the whole escape sequence by a
  // variable named something like '?bnd123'.
  anf::convertToANF(pctx.prog.get(), false);
#ifdef _DEBUG
  std::cout << to_string(*pctx.prog.get()) << std::endl;
#endif

  // find the escapes expressions in the R AST, and put them in a
  // vector so we can evaluate them:
  std::vector<SEXP> bndExpr;
  findEscape(e, bndExpr);

  // get the bound variable names (e.g. '?bnd123' which were inserted by the
  // call to 'convertToANF'):
  set<string> bndvarNames;
  getBoundVars(pctx.prog.get(), bndvarNames);

  // evaluate the bound expressions (in the calling environment passed
  // in 'env') and put them in a list that will be passed to 'sendReq':
  Rcpp::Environment current(env);
  if (bndExpr.size() != bndvarNames.size()) {
    Rcpp::stop("bound variable count mismatch");
  }
  auto bndvarList = arr::make_cow<val::VList>(false);
  size_t i = 0;
  for (auto name : bndvarNames) {
    auto evalResult = Rf_eval(bndExpr[i++], current);
#ifdef _DEBUG
    std::cout << "eval result: " << std::endl;
    Rcpp::print(evalResult); 
#endif

    // convert it to 'Value' and put it in the list
    bndvarList->push_back(std::make_pair(name, SEXPToValue(evalResult)));
  }
  
  res = net->sendReq(peer_conn_id, 0, pctx.prog.get(), bndvarList);
  net->run();                   // poll for the response data
  
  return valueToSEXP(net->getVal());
}


std::unique_ptr<net::NetHandler> Zconnection::com = nullptr;
std::unique_ptr<client::Client> Zconnection::net = nullptr;
volatile bool Zconnection::stop = 0;


RCPP_MODULE(ZconnectionEx) {
  using namespace Rcpp ;

  class_<Zconnection>( "Zconnection" )

  .constructor<std::string, int>()
  .method("query", &Zconnection::query) 
  ;
}

