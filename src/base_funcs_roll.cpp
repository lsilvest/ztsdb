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


#include <algorithm>
#include "array_ops.hpp"
#include "zts.hpp"
#include "base_funcs.hpp"
#include "timezone/ztime_vector.hpp"
#include "misc.hpp"


static void checkParams(const arr::Array<double>& a, double window, double nbvalid,
                        const yy::location& a_loc, const yy::location& window_loc,
                        const yy::location& nbvalid_loc) {
  if (a.size() == 0) {
    throw interp::EvalException("matrix has 0 elements", a_loc);
  }
  if (window <= 0 || window > a.dim[0]) {
    throw interp::EvalException("'window' must be >= 0 and <= dim[0]", window_loc);
  }
  if (nbvalid <= 1 || nbvalid > window) {
    throw interp::EvalException("'nbvalid' must be >= 1 and <= window", nbvalid_loc);
  }
}


template <arr::Array<double>& (*rollfunc3)(arr::Array<double>&, arr::idx_type, arr::idx_type)>
static inline val::Value doroll(const vector<val::VBuiltinG::arg_t>& v) {
  enum {X, WINDOW, NVALID};
  const size_t window  = funcs::getUint(val::get_scalar<double>(getVal(v[WINDOW])),
                                        getLoc(v[WINDOW]));
  const size_t nbvalid = funcs::getUint(val::get_scalar<double>(getVal(v[NVALID])),
                                        getLoc(v[NVALID]));
  
  switch (getVal(v[0]).which()) {
  case val::vt_double: {
    const auto& aconst = get<val::SpVAD>(getVal(v[X]));
    checkParams(*aconst, window, nbvalid, getLoc(v[X]), getLoc(v[WINDOW]),  getLoc(v[NVALID]));
    auto a = get<val::SpVAD>(getVal(v[X]));
    rollfunc3(*a, window, nbvalid); // copy when not ref
    return a;
  }
  case val::vt_zts: {
    const auto& zconst = get<val::SpZts>(getVal(v[X]));
    checkParams(zconst->getArray(), window, nbvalid,
                getLoc(v[X]), getLoc(v[WINDOW]),  getLoc(v[NVALID]));
    auto z = get<val::SpZts>(getVal(v[X]));
    rollfunc3(*z->getArrayPtr(), window, nbvalid); // copy when not ref
    return z;
  }
  default:
    throw out_of_range("invalid argument type");
  }
}


val::Value funcs::rollmean(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return doroll<rollmean_inplace>(v);
}

val::Value funcs::rollmin(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return doroll<rollmin_inplace>(v);
}

val::Value funcs::rollmax(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return doroll<rollmax_inplace>(v);
}

val::Value funcs::rollvar(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return doroll<rollvar_inplace>(v);
}

val::Value funcs::rollcov(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum {X, Y, WINDOW, NVALID};
  const size_t window  = funcs::getUint(val::get_scalar<double>(getVal(v[WINDOW])),
                                        getLoc(v[WINDOW]));
  const size_t nbvalid = funcs::getUint(val::get_scalar<double>(getVal(v[NVALID])),
                                        getLoc(v[NVALID]));

  bool isXdouble = getVal(v[X]).which() == val::vt_double;
  bool isYdouble = getVal(v[Y]).which() == val::vt_double;
  
  const auto& xconst = isXdouble ?
    *static_cast<const val::SpVAD>(get<val::SpVAD>(getVal(v[X]))) :
    get<val::SpZts>(getVal(v[X]))->getArray();
  const auto& yconst = isYdouble ? 
    *static_cast<const val::SpVAD>(get<val::SpVAD>(getVal(v[Y]))) :
    get<val::SpZts>(getVal(v[Y]))->getArray();
  checkParams(xconst, window, nbvalid, getLoc(v[X]), getLoc(v[WINDOW]),  getLoc(v[NVALID]));
  checkParams(yconst, window, nbvalid, getLoc(v[Y]), getLoc(v[WINDOW]),  getLoc(v[NVALID]));

  if (isXdouble && isYdouble) {
    return arr::make_cow<val::VArrayD>(false, rollcov(xconst, yconst, window, nbvalid));
  }
  else if (!isXdouble) {
    return arr::make_cow<arr::zts>(false,
                                   get<val::SpZts>(getVal(v[X]))->getIndex(),
                                   rollcov(xconst, yconst, window, nbvalid));
  }
  else {
    return arr::make_cow<arr::zts>(false,
                                   get<val::SpZts>(getVal(v[Y]))->getIndex(),
                                   rollcov(xconst, yconst, window, nbvalid));
  }
}


template <arr::Array<double>& (*rollfunc2)(arr::Array<double>&, ssize_t)>
static inline val::Value doroll2(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum {X, N};
  const size_t n  = funcs::getInt(val::get_scalar<double>(getVal(v[N])), getLoc(v[N]));
  
  switch (getVal(v[X]).which()) {
  case val::vt_double: {
    val::SpVAD a = get<val::SpVAD>(getVal(v[X]));
    rollfunc2(*a, n);           // potentially a copy when not ref
    return a;
  }
  case val::vt_zts: {
    auto z = get<val::SpZts>(getVal(v[X]));
    rollfunc2(*z->getArrayPtr(), n); // potentially a copy when not ref
    return z;
  }
  default:
    throw interp::EvalException("invalid argument type", getLoc(v[X]));
  }
}


val::Value funcs::locf(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return doroll2<arr::locf_inplace>(v, ic);
}


val::Value funcs::move(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return doroll2<arr::move_inplace>(v, ic);
}


val::Value funcs::rotate(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum {X, N};
  const size_t n  = funcs::getInt(val::get_scalar<double>(getVal(v[N])), getLoc(v[N]));
  
  switch (getVal(v[X]).which()) {
  case val::vt_double: {
    val::SpVAD a = get<val::SpVAD>(getVal(v[X]));
    arr::rotate_inplace(*a, n);           // potentially a copy when not ref
    return a;
  }
  case val::vt_time: {
    val::SpVADT a = get<val::SpVADT>(getVal(v[X]));
    arr::rotate_inplace(*a, n);           // potentially a copy when not ref
    return a;
  }
  // correct this!
  // case val::vt_bool: {
  //   val::SpVAB a = get<val::SpVAB>(getVal(v[X]));
  //   arr::rotate_inplace(*a, n);           // potentially a copy when not ref
  //   return a;
  // }
  case val::vt_duration: {
    val::SpVADUR a = get<val::SpVADUR>(getVal(v[X]));
    arr::rotate_inplace(*a, n);           // potentially a copy when not ref
    return a;
  }
  case val::vt_interval: {
    val::SpVAIVL a = get<val::SpVAIVL>(getVal(v[X]));
    arr::rotate_inplace(*a, n);           // potentially a copy when not ref
    return a;
  }
  case val::vt_zts: {
    auto z = get<val::SpZts>(getVal(v[X]));
    arr::rotate_inplace(*z->getArrayPtr(), n); // potentially a copy when not ref
    return z;
  }
  default:
    throw interp::EvalException("invalid argument type", getLoc(v[X]));
  }
}


val::Value funcs::diff(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return doroll2<arr::diff_inplace>(v, ic);
}


template <arr::Array<double>& (*cumfunc)(arr::Array<double>&, bool)>
static inline val::Value docum(const vector<val::VBuiltinG::arg_t>& v) {
  enum {X, REV};
  const bool rev = val::get_scalar<bool>(getVal(v[REV]));

  switch (getVal(v[X]).which()) {
  case val::vt_double: {
    auto a = get<val::SpVAD>(getVal(v[X]));
    cumfunc(*a, rev); // copy when not ref
    return a;
  }
  case val::vt_zts: {
    auto z = get<val::SpZts>(getVal(v[X]));
    cumfunc(*z->getArrayPtr(), rev); // copy when not ref
    return z;
  }
  default:
    throw interp::EvalException("invalid argument type", getLoc(v[X]));
  }
}


val::Value funcs::cumsum(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return docum<arr::cumul_inplace<double, std::plus<double>>>(v);
}


val::Value funcs::cumprod(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return docum<arr::cumul_inplace<double, std::multiplies<double>>>(v);
}


val::Value funcs::cumdiv(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return docum<arr::cumul_inplace<double, std::divides<double>>>(v);
}


val::Value funcs::cummax(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return docum<arr::cumul_inplace<double, ztsdb::max<double>>>(v);
}


val::Value funcs::cummin(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return docum<arr::cumul_inplace<double, ztsdb::min<double>>>(v);
}


template<typename T>
struct rev_wrapper {
  static val::Value f(const val::Value& val, const yy::location& loc_val) {
    auto v = get<T>(val);
    arr::rev_inplace(*v);
    return v;
  }
};

val::Value funcs::rev(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  if (getVal(v[0]).which() == val::vt_zts) {
    auto z = get<val::SpZts>(getVal(v[0]));
    arr::rev_inplace(*z->getArrayPtr());
    return z;
  }
  else if (getVal(v[0]).which() == val::vt_list) {
    auto l = get<val::SpVList>(getVal(v[0]));
    arr::rev_inplace(l->a);
    return l;
  }
  else {
    return apply_to_types<rev_wrapper,
                          val::vt_double, 
                          val::vt_bool, 
                          val::vt_time, 
                          val::vt_string, 
                          val::vt_duration, 
                          val::vt_interval>(getVal(v[0]), getLoc(v[0]));
  }
}


template <typename T, typename F>
struct Aggr {
  Aggr(T i): res{i} { }
  void operator()(T n) { res = F()(res, n); }
  T res;
};

val::Value funcs::sum(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  if (getVal(v[0]).which() == val::vt_zts) {
    const auto& z = get<val::SpZts>(getVal(v[0]));
    auto s = z->getArray().for_each(Aggr<double, std::plus<double>>(0.0));
    return val::make_array(s.res);
  }
  else if (getVal(v[0]).which() == val::vt_double) {
    const auto& a = get<val::SpVAD>(getVal(v[0]));
    auto s = a->for_each(Aggr<double, std::plus<double>>(0.0));
    return val::make_array(s.res);
  }
  else if (getVal(v[0]).which() == val::vt_duration) {
    const auto& a = get<val::SpVADUR>(getVal(v[0]));
    auto s = a->for_each(Aggr<Global::duration, std::plus<Global::duration>>(0s));
    return val::make_array(s.res);
  }
  else {
    throw interp::EvalException("invalid type", getLoc(v[0]));
  }
}

val::Value funcs::prod(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  if (getVal(v[0]).which() == val::vt_zts) {
    const auto& z = get<val::SpZts>(getVal(v[0]));
    auto s = z->getArray().for_each(Aggr<double, std::multiplies<double>>(1.0));
    return val::make_array(s.res);
  }
  else if (getVal(v[0]).which() == val::vt_double) {
    const auto& a = get<val::SpVAD>(getVal(v[0]));
    auto s = a->for_each(Aggr<double, std::multiplies<double>>(1.0));
    return val::make_array(s.res);
  }
  else {
    throw interp::EvalException("invalid type", getLoc(v[0]));
  }
}



template <class F>
static val::Value minMax(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {

}

template <typename T>
struct Min {
  T operator()(const T& t1, const T& t2) { return std::min(t1, t2); }
};

val::Value funcs::_min(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  if (getVal(v[0]).which() == val::vt_zts) {
    const auto& z = get<val::SpZts>(getVal(v[0]));
    auto s = z->getArray().for_each(Aggr<double,
                                    Min<double>>(std::numeric_limits<double>::max()));
    return val::make_array(s.res);
  }
  else if (getVal(v[0]).which() == val::vt_double) {
    const auto& a = get<val::SpVAD>(getVal(v[0]));
    auto s = a->for_each(Aggr<double, Min<double>>(std::numeric_limits<double>::max()));
    return val::make_array(s.res);
  }
  else if (getVal(v[0]).which() == val::vt_duration) {
    const auto& a = get<val::SpVADUR>(getVal(v[0]));
    auto s = a->for_each(Aggr<Global::duration,
                         Min<Global::duration>>(std::numeric_limits<Global::duration>::max()));
    return val::make_array(s.res);
  }
  else {
    throw interp::EvalException("invalid type", getLoc(v[0]));
  }
}


template <typename T>
struct Max {
  T operator()(const T& t1, const T& t2) { return std::max(t1, t2); }
};

val::Value funcs::_max(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  if (getVal(v[0]).which() == val::vt_zts) {
    const auto& z = get<val::SpZts>(getVal(v[0]));
    auto s = z->getArray().for_each(Aggr<double,
                                    Max<double>>(std::numeric_limits<double>::min()));
    return val::make_array(s.res);
  }
  else if (getVal(v[0]).which() == val::vt_double) {
    const auto& a = get<val::SpVAD>(getVal(v[0]));
    auto s = a->for_each(Aggr<double, Max<double>>(std::numeric_limits<double>::min()));
    return val::make_array(s.res);
  }
  else if (getVal(v[0]).which() == val::vt_duration) {
    const auto& a = get<val::SpVADUR>(getVal(v[0]));
    auto s = a->for_each(Aggr<Global::duration,
                         Max<Global::duration>>(std::numeric_limits<Global::duration>::min()));
    return val::make_array(s.res);
  }
  else {
    throw interp::EvalException("invalid type", getLoc(v[0]));
  }
}
