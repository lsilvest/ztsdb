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

// For the algo's we use a modified version of Howard Hinnant's code
// in "date.h" (http://howardhinnant.github.io/date_v2.html with the
// algos further explained here:
// http://howardhinnant.github.io/date_algorithms.html). The license
// for this modified code is:

// The MIT License (MIT)
// 
// Copyright (c) 2015 Howard Hinnant
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


#include <ztsdb/globals.hpp>
#include "zconversion.hpp"


static Rcpp::List convertDimnames(const std::vector<std::unique_ptr<arr::Dname>>& dnames) {
  Rcpp::List l;
  for (auto& n : dnames) {
    Rcpp::CharacterVector cv;
    for (auto& e : n->names) {
      cv.push_back(e);
    }
    l.push_back(cv);
  }
  return l;
}


template <int RTYPE, typename T>
static Rcpp::Vector<RTYPE> convertArray(const arr::Array<T>& a) {
  // if we have a vector we can do 'memcpy' LLL
  Rcpp::Vector<RTYPE> v;
  for (arr::idx_type i=0; i<a.size(); ++i) {
    v.push_back(a[i]);
  }
  v.attr("dim") = Rcpp::IntegerVector(a.getdim().begin(), a.getdim().end());
  v.attr("dimnames") = convertDimnames(a.names);
  return v;
}


static Rcpp::NumericVector convertDtimeArray(const arr::Array<Global::dtime>& a) {
  Rcpp::NumericVector v;
  for (arr::idx_type i=0; i<a.size(); ++i) {
    auto dt = a[i];
    double d = dt.time_since_epoch().count() / 1e9;
    auto r = dt.time_since_epoch().count() % 1000; // verify that we are not rounding
    Rcpp::Function warning("warning");
    if (r) {
      warning("time precision loss");
    }
    v.push_back(d);
  }
  v.attr("class") = Rcpp::CharacterVector{"POSIXct", "POSIXt"};
  v.attr("dim") = Rcpp::IntegerVector(a.getdim().begin(), a.getdim().end());
  v.attr("dimnames") = convertDimnames(a.names);
  return v;
}


static Rcpp::NumericVector convertDtimeVector(const Vector<Global::dtime>& a) {
  Rcpp::NumericVector v;
  for (arr::idx_type i=0; i<a.size(); ++i) {
    auto dt = a[i];
    double d = dt.time_since_epoch().count() / 1e9;
    auto r = dt.time_since_epoch().count() % 1000; // verify that we are not rounding
    Rcpp::Function warning("warning");
    if (r) {
      warning("time precision loss");
    }
    v.push_back(d);
  }
  v.attr("class") = Rcpp::CharacterVector{"POSIXct", "POSIXt"};
  return v;
}

union int64_double {
  int64_t i;
  double d;
};


static Rcpp::NumericVector convertDurationArray(const arr::Array<Global::duration>& a) {
  Rcpp::NumericVector v;
  for (arr::idx_type i=0; i<a.size(); ++i) {
    int64_double id;
    id.i = a[i].count();
    v.push_back(id.d);
  }
  v.attr("class") = Rcpp::CharacterVector{"integer64"};
  v.attr("dim") = Rcpp::IntegerVector(a.getdim().begin(), a.getdim().end());
  v.attr("dimnames") = convertDimnames(a.names);
  return v;
}


static Rcpp::NumericVector convertZts(const arr::zts& z) {
  // for this function, we just set following the output on the R
  // command line of 'attributes(xts_object)'.

  // we see that an 'xts' is a 'NumericVector' with an index as
  // attribute 'index'; the following takes care of attributes {dim, dimnames}:
  Rcpp::NumericVector xts = convertArray<REALSXP, double>(z.getArray());

  // now set the index:
  xts.attr("index") = convertDtimeVector(z.getIndexPtr()->getcol(0));

  // and the remaining 
  xts.attr("tzone") = Rcpp::CharacterVector{""};
  xts.attr("tclass") = Rcpp::CharacterVector{"POSIXct", "POSIXt"};
  xts.attr(".indexCLASS") = Rcpp::CharacterVector{"POSIXct", "POSIXt"};
  xts.attr(".indexTZ") = Rcpp::CharacterVector{""};
  xts.attr("descr") = Rcpp::CharacterVector{""};
  xts.attr("class") = Rcpp::CharacterVector{"xts", "zoo"};
  return xts;
}


SEXP valueToSEXP(const val::Value& v) {
  switch (v.which()) {
  case val::vt_zts: {
    val::SpZts ts = get<val::SpZts>(v);
    Rcpp::NumericVector v = convertZts(*ts);
    return Rcpp::wrap(v);
  }
  case val::vt_double: {
    val::SpVAD a = get<val::SpVAD>(v);
    Rcpp::NumericVector v = convertArray<REALSXP, double>(*a);
    return Rcpp::wrap(v);
  }
  case val::vt_bool: {
    val::SpVAB a = get<val::SpVAB>(v);
    Rcpp::LogicalVector v = convertArray<LGLSXP, bool>(*a);
    return Rcpp::wrap(v);
  }
  case val::vt_time: {
    val::SpVADT a = get<val::SpVADT>(v);
    Rcpp::NumericVector v = convertDtimeArray(*a);
    return Rcpp::wrap(v);
  }
  case val::vt_duration: {
    val::SpVADUR a = get<val::SpVADUR>(v);
    Rcpp::NumericVector v = convertDurationArray(*a);
    return Rcpp::wrap(v);
  }
  case val::vt_string: {
    val::SpVAS a = get<val::SpVAS>(v);
    Rcpp::CharacterVector v = convertArray<STRSXP, arr::zstring>(*a);
    return Rcpp::wrap(v);
  }
  case val::vt_error: {
    auto e = get<val::VError>(v);
    throw std::range_error(e.what);
  }
  default:
    std::cout << "return value: " << v.which() << std::endl;
    throw std::range_error("unkown return value");
  }
}


