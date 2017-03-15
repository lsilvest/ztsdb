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


#include <dlfcn.h>
#include <string>
#include <iterator>
#include <exception>
#include <system_error>
#include <type_traits>
#include <cstring>
#include "base_funcs.hpp"
#include "conversion_funcs.hpp"
#include "display.hpp"
#include "csv.hpp"
#include "array_ops.hpp"
#include "env.hpp"
#include "zts.hpp"
#include "timezone/zone.hpp"
#include "timezone/ztime.hpp"
#include "timezone/ztime_vector.hpp"
#include "hinnant_date/date.h"
#include "period.hpp"
#include "align_funcs.hpp"
#include "pseudoarray.hpp"


extern tz::Zones tzones;


// all/any: -----------------------------------
template<template <class> class F, bool INIT, bool END>
static val::Value any_all(vector<val::VBuiltinG::arg_t>& v) {
  bool r = INIT;
  for (const auto& e : v) {
    switch (get<1>(e).which()) {
    case val::vt_double: 
      r = F<bool>()(r, arr::cumul_until<double, bool, F>(*get<val::SpVAD>(get<1>(e)), INIT, END));
      break;
    case val::vt_bool: 
      r = F<bool>()(r, arr::cumul_until<bool, bool, F>(*get<val::SpVAB>(get<1>(e)), INIT, END));
      break;
    // case val::vt_arrays: 
    // case val::vt_arraydt: 
    // case val::vt_string:
    // case val::vt_time:
    // case val::vt_duration:
    // case val::vt_interval:
    // case val::vt_null:          // like in R, NULLs are just ignored in 'c'
    //   break;
    default:
      throw std::domain_error("can't coherce to logical");
    }
  }
  return val::make_array(r);  
}

val::Value funcs::all(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return any_all<std::logical_and, true, false>(v);
}

val::Value funcs::any(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return any_all<std::logical_or, false, true>(v);
}

val::Value funcs::is_nan(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {  
  const auto& d = get<val::SpVAD>(get<1>(v[0]));
  return make_cow<val::VArrayB>(false, applyf<double, bool>(*d, [](double u) { return std::isnan(u); }));
}

val::Value funcs::is_infinite(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  const auto& d = get<val::SpVAD>(get<1>(v[0]));
  return make_cow<val::VArrayB>(false, applyf<double, bool>(*d, [](double u) { return std::isinf(u); }));
}


val::Value funcs::all_equal(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { X, Y };
  return val::make_array(val::getVal(v[X]) == val::getVal(v[Y]));
}


// as.xxx conversion functions: -----------------------------------
val::Value funcs::as_logical(vector<val::VBuiltinG::arg_t>& v, 
                             zcore::InterpCtx& ic) {
  return convert_logical(val::getVal(v[0]));
}

val::Value funcs::as_numeric(vector<val::VBuiltinG::arg_t>& v, 
                             zcore::InterpCtx& ic) {
  return convert_numeric(val::getVal(v[0]));
}

val::Value funcs::as_character(vector<val::VBuiltinG::arg_t>& v, 
                               zcore::InterpCtx& ic) {
  return convert_character(val::getVal(v[0]));
}

val::Value funcs::as_duration(vector<val::VBuiltinG::arg_t>& v, 
                              zcore::InterpCtx& ic) {
  return convert_duration(val::getVal(v[0]));
}

val::Value funcs::as_period(vector<val::VBuiltinG::arg_t>& v, 
                            zcore::InterpCtx& ic) {
  return convert_period(val::getVal(v[0]));
}

val::Value funcs::as_time(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return convert_time(val::getVal(v[0]));  
}

val::Value funcs::as_interval(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return convert_interval(val::getVal(v[0]));  
}

val::Value funcs::make_time(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { YEAR, MONTH, DAY, HOUR, MINUTE, SECOND, NS, TZ };
  const auto& y     = get<val::SpVAD>(val::getVal(v[YEAR]));
  const auto& m     = get<val::SpVAD>(val::getVal(v[MONTH]));
  const auto& d     = get<val::SpVAD>(val::getVal(v[DAY]));

  // check sizes for y, m, d:
  if (y->getdim() != m->getdim()) 
    throw interp::EvalException("array size mismatch", val::getLoc(v[MONTH]));
  if (y->getdim() != d->getdim()) 
    throw interp::EvalException("array size mismatch", val::getLoc(v[DAY]));

  // any of these can be null, if so, create an array of same size as y, with all elts == 0:
  const auto& h = val::getVal(v[HOUR]).which() == val::vt_double ? 
    get<val::SpVAD>(val::getVal(v[HOUR])) : 
    make_cow<arr::Array<double>>(false, y->getdim(), arr::Vector<double>());
  const auto& mn = val::getVal(v[MINUTE]).which() == val::vt_double ? 
    get<val::SpVAD>(val::getVal(v[MINUTE])) : 
    make_cow<arr::Array<double>>(false, y->getdim(), arr::Vector<double>());
  const auto& s  = val::getVal(v[SECOND]).which() == val::vt_double ? 
    get<val::SpVAD>(val::getVal(v[SECOND])) : 
    make_cow<arr::Array<double>>(false, y->getdim(), arr::Vector<double>());
  const auto& ns = val::getVal(v[NS]).which() == val::vt_double ? 
    get<val::SpVAD>(val::getVal(v[NS])) : 
    make_cow<arr::Array<double>>(false, y->getdim(), arr::Vector<double>());

  // can be either an array or a scalar:
  const auto& tzstr = get<val::SpVAS>(val::getVal(v[TZ]));

  // check the rest of the sizes:
  if (y->getdim() != h->getdim()) 
    throw interp::EvalException("array size mismatch", val::getLoc(v[HOUR]));
  if (y->getdim() != mn->getdim()) 
    throw interp::EvalException("array size mismatch", val::getLoc(v[MONTH]));
  if (y->getdim() != s->getdim()) 
    throw interp::EvalException("array size mismatch", val::getLoc(v[MONTH]));
  if (y->getdim() != ns->getdim()) 
    throw interp::EvalException("array size mismatch", val::getLoc(v[MONTH]));

  // not ideal: we initialize the array and then overwrite:
  auto ret = make_cow<arr::Array<Global::dtime>>
    (false, arr::Array<Global::dtime>(y->getdim(), Vector<Global::dtime>())); 

  if (tzstr->isScalar()) {
    auto& tz = tzones.find((*tzstr)[0]);
    for (size_t i=0; i<ret->size(); ++i) {
      arr::setv(*ret, i, tz::dtime_from_numbers((*y)[i], (*m)[i], (*d)[i], (*h)[i], (*mn)[i], 
                                                 (*s)[i], (*ns)[i], tz));
    }
  }
  else {
    if (y->getdim() != tzstr->getdim()) 
      throw interp::EvalException("array size mismatch", val::getLoc(v[TZ]));
    for (size_t i=0; i<ret->size(); ++i) {
      auto& tz = tzones.find((*tzstr)[i]);
      arr::setv(*ret, i, tz::dtime_from_numbers((*y)[i], (*m)[i], (*d)[i], (*h)[i], (*mn)[i], 
                                                (*s)[i], (*ns)[i], tz));
    }
  }

  return ret;
}


struct period_from_numbers {
  tz::period operator()(double month, double day, Global::duration dur) {
    return tz::period(month, day, dur);
  }
};

val::Value funcs::make_period(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { MONTH, DAY, DURATION };
  const auto& month = get<val::SpVAD>(val::getVal(v[MONTH]));
  const auto& day   = get<val::SpVAD>(val::getVal(v[DAY]));
  const auto& dur   = get<val::SpVADUR>(val::getVal(v[DURATION]));
  
  // LLL catch the exception to say we can't build the period
  return make_cow<arr::Array<tz::period>>
    (false, apply<
     period_from_numbers, tz::period, Array<tz::period>::comparator,
     Array<double>, Array<double>, Array<Global::duration>>
     (*month, *day, *dur));
} 

val::Value funcs::period_month(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  const auto& p = get<val::SpVAPRD>(val::getVal(v[0]));
  return make_cow<arr::Array<double>>
    (false, arr::applyf<tz::period, double>
     (*p, [](tz::period u) { return static_cast<double>(u.getMonths()); }));
}

val::Value funcs::period_day(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  const auto& p = get<val::SpVAPRD>(val::getVal(v[0]));
  return make_cow<arr::Array<double>>
    (false, arr::applyf<tz::period, double>
     (*p, [](tz::period u) { return static_cast<double>(u.getDays()); }));
}

val::Value funcs::period_duration(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  const auto& p = get<val::SpVAPRD>(val::getVal(v[0]));
  return make_cow<arr::Array<Global::duration>>
    (false, arr::applyf<tz::period, Global::duration>
     (*p, [](tz::period u) { return u.getDuration(); }));
}

val::Value funcs::character(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  auto length = val::get_scalar<double>(val::getVal(v[0]));
  if (length < 0) {
    throw interp::EvalException("invalid 'length' argument", val::getLoc(v[0]));
  }
  return make_cow<val::VArrayS>(false, rsv, Vector<idx_type>{static_cast<size_t>(length)});
}


struct interval_wrapper {
  tz::interval operator()(Global::dtime s, Global::dtime e, uint32_t sopen, uint32_t eopen) {
    return tz::interval(s, e, sopen, eopen);
  }
};


val::Value funcs::_interval(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { START, END, DURATION, SOPEN, EOPEN };
  const auto& start = get<val::SpVADT>(val::getVal(v[START]));
  const auto& end_t = val::getVal(v[END]).which();
  const auto& duration_t = val::getVal(v[DURATION]).which();
  const auto& sopen = get<val::SpVAB>(val::getVal(v[SOPEN]));
  const auto& eopen = get<val::SpVAB>(val::getVal(v[EOPEN]));
  if (end_t != val::vt_null && duration_t != val::vt_null) {
    throw interp::EvalException("'end' and 'duration' cannot be specified together", val::getLoc(v[END]));
  }
  if (sopen->size() != 1 && sopen->size() != start->size()) {
    throw interp::EvalException("'sopen' size must be 1 or the same as 'start'", 
                                val::getLoc(v[SOPEN]));
  }
  if (eopen->size() != 1 && eopen->size() != start->size()) {
    throw interp::EvalException("'eopen' size must be 1 or the same as 'start'", 
                                val::getLoc(v[EOPEN]));
  }
  const PseudoArray<bool> sopen_pv(*sopen, start->ncols(), start->size());
  const PseudoArray<bool> eopen_pv(*eopen, start->ncols(), start->size());

  if (end_t == val::vt_time) {
    auto end = get<val::SpVADT>(val::getVal(v[END]));
    if (start->size() != end->size()) {
      throw interp::EvalException("'start' and 'end' must have the same size", val::getLoc(v[END]));
    }

    return arr::make_cow<val::VArrayIVL>
      (false, 
       arr::apply<interval_wrapper, 
                  tz::interval, Vector<tz::interval>::comparator, 
                  val::VArrayDT, 
                  val::VArrayDT, 
                  PseudoArray<bool>, 
                  PseudoArray<bool>>(*start, *end, sopen_pv, eopen_pv));
  }
  else {
    auto duration = get<val::SpVADUR>(val::getVal(v[DURATION]));
    if (duration->size() != 1 && duration->size() != start->size()) {
      throw interp::EvalException("'duration' size must be 1 or the same as 'start'", 
                                  val::getLoc(v[DURATION]));
    }

    const PseudoArray<Global::duration> duration_pv(*duration, start->ncols(), start->size());
    const auto end = arr::apply<ztsdb::plus<Global::dtime,Global::duration,Global::dtime>,
                                Global::dtime, 
                                Array<Global::dtime>::comparator,
                                val::VArrayDT, 
                                PseudoArray<Global::duration>>(*start, duration_pv);

    return arr::make_cow<val::VArrayIVL>
      (false, 
       arr::apply<interval_wrapper, 
                  tz::interval, Vector<tz::interval>::comparator, 
                  val::VArrayDT, 
                  val::VArrayDT, 
                  PseudoArray<bool>, 
                  PseudoArray<bool>>(*start, end, sopen_pv, eopen_pv));
  }
}


val::Value funcs::interval_start(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  const auto& i = get<val::SpVAIVL>(val::getVal(v[0]));
  return make_cow<arr::Array<Global::dtime>>(false, arr::applyf<tz::interval, Global::dtime>
                                             (*i, [](tz::interval u) { return u.s; }));
}

val::Value funcs::interval_end(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  const auto& i = get<val::SpVAIVL>(val::getVal(v[0]));
  return make_cow<arr::Array<Global::dtime>>(false, arr::applyf<tz::interval, Global::dtime>
                                             (*i, [](tz::interval u) { return u.e; }));
}

val::Value funcs::interval_sopen(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  const auto& i = get<val::SpVAIVL>(val::getVal(v[0]));
  return make_cow<arr::Array<bool>>(false, arr::applyf<tz::interval, bool>
                                        (*i, [](tz::interval u) { return u.sopen; }));
}

val::Value funcs::interval_eopen(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  const auto& i = get<val::SpVAIVL>(val::getVal(v[0]));
  return make_cow<arr::Array<bool>>(false, arr::applyf<tz::interval, bool>
                                        (*i, [](tz::interval u) { return u.eopen; }));
}


val::Value funcs::vlist(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) { 
  val::VList l;
  for (auto& e : v) {
    l.a.concat(val::getVal(e), val::getName(e));
  }
  return make_cow<val::VList>(arr::NOFLAGS, l); 
} 


val::Value funcs::is_null(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return val::make_array(val::getVal(v[0]).which() == val::vt_null);
}


val::Value funcs::get_typeof(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return val::make_array<arr::zstring>(apply_visitor(val::Typeof(), val::getVal(v[0])));
}


val::Value funcs::dyn_load(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  // arg 0: filename
  // arg 1: local: nothing (true), RTLD_GLOBAL (false)
  // arg 2: now:   RTLD_NOW (true), RTLD_LAZY (false)

  bool local = val::get_scalar<bool>(val::getVal(v[1]));
  bool now   = val::get_scalar<bool>(val::getVal(v[2]));

  int flag = (local ? 0 : RTLD_GLOBAL) | (now ? RTLD_NOW : RTLD_LAZY);
  void* handle = dlopen(val::get_scalar<arr::zstring>(val::getVal(v[0])).c_str(), flag);
  if (!handle) {
    throw std::system_error(std::error_code(errno, std::system_category()), "dlopen");    
  }
 
  return val::VNull();                  // ? LLL check what R does
}


val::Value funcs::quit(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  auto status = static_cast<int>(val::get_scalar<double>(val::getVal(v[0])));
  throw Global::QuitException(status);
}


val::Value funcs::read_csv(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum {CSVFILE, TYPE, ARRAYFILE, HEADER, SEP, FORMAT, TZ};
  // row.names: not yet implemented
  // col.names: not yet implemented
  const string csvfile = val::get_scalar<arr::zstring>(val::getVal(v[CSVFILE]));
  const string type = val::get_scalar<arr::zstring>(val::getVal(v[TYPE]));
  const string arrayfile = val::get_scalar<arr::zstring>(val::getVal(v[ARRAYFILE]));
  auto hasHeader = val::get_scalar<bool>(val::getVal(v[HEADER]));
  const string sep = val::get_scalar<arr::zstring>(val::getVal(v[SEP]));
  string format = val::get_scalar<arr::zstring>(val::getVal(v[FORMAT]));
  const string tz = val::get_scalar<arr::zstring>(val::getVal(v[TZ]));
  // in R:
  // col.names can be:
  // -- NULL
  // -- vector<zstring> 
  // row.names can be
  // -- NULL -> make them numeric (but we implement no names)
  // -- one string, names of column that has names
  // -- one number, number of column that has names
  // -- vector<zstring>
  if (type == "double") {
    return arr::readcsv_array<double>(csvfile, hasHeader, sep[0], arrayfile);
  }
  else if (type == "zts") {
    return arr::readcsv_zts(csvfile, hasHeader, sep[0], arrayfile, format, tz);
  }
  else if (type == "logical") {
    return arr::readcsv_array<bool>(csvfile, hasHeader, sep[0], arrayfile);
  }
  else if (type == "time") {
    return arr::readcsv_array<Global::dtime>(csvfile, hasHeader, sep[0], arrayfile);
  }
  else if (type == "character") {
    return arr::readcsv_array<arr::zstring>(csvfile, hasHeader, sep[0], arrayfile);
  }
  else if (type == "interval") {
    return arr::readcsv_array<tz::interval>(csvfile, hasHeader, sep[0], arrayfile);
  }
  else if (type == "period") {
    return arr::readcsv_array<tz::period>(csvfile, hasHeader, sep[0], arrayfile);
  }
  else {
    throw range_error("unknown type " + type);
  }
}


val::Value funcs::write_csv(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum {OBJECT, CSVFILE, HEADER, SEP};
  const string csvfile = val::get_scalar<arr::zstring>(val::getVal(v[CSVFILE]));
  auto doHeader = val::get_scalar<bool>(val::getVal(v[HEADER]));
  const string sep = val::get_scalar<arr::zstring>(val::getVal(v[SEP]));
  if (sep.size() != 1) {
    throw interp::EvalException("separator must be exactly one character long", val::getLoc(v[SEP]));
  }
  switch (val::getVal(v[OBJECT]).which()) {
  case val::vt_zts: {
    const auto ai = get<const val::SpZts>(val::getVal(v[OBJECT]));
    arr::writecsv_zts(*ai, csvfile, doHeader, sep[0]);
    break;    
  }
  case val::vt_double: {
    const auto ai = get<const val::SpVAD>(val::getVal(v[OBJECT]));
    arr::writecsv_array(*ai, csvfile, doHeader, sep[0]);
    break;
  }
  case val::vt_time: {
    const auto ai = get<const val::SpVADT>(val::getVal(v[OBJECT]));
    arr::writecsv_array(*ai, csvfile, doHeader, sep[0]);
    break;    
  }
  case val::vt_bool: {
    const auto ai = get<const val::SpVAB>(val::getVal(v[OBJECT]));
    arr::writecsv_array(*ai, csvfile, doHeader, sep[0]);
    break;    
  }
  case val::vt_string: {
    const auto ai = get<const val::SpVAS>(val::getVal(v[OBJECT]));
    arr::writecsv_array(*ai, csvfile, doHeader, sep[0]);
    break;    
  }
  case val::vt_interval: {
    const auto ai = get<const val::SpVAIVL>(val::getVal(v[OBJECT]));
    arr::writecsv_array(*ai, csvfile, doHeader, sep[0]);
    break;    
  }
  case val::vt_period: {
    const auto ai = get<const val::SpVAPRD>(val::getVal(v[OBJECT]));
    arr::writecsv_array(*ai, csvfile, doHeader, sep[0]);
    break;    
  }
  default:
    throw range_error("can't write csv file for type " + apply_visitor(val::Typeof(), val::getVal(v[0])));
  } 
  return val::VNull();
}
  

val::Value funcs::substr(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { X, START, STOP };
  auto     x = get<val::SpVAS>(val::getVal(v[X]));
  auto start = val::get_scalar<double>(val::getVal(v[START]))-1;
  if (start < 0) start = 0;
  auto stop  = val::get_scalar<double>(val::getVal(v[STOP]));

  std::function<arr::zstring(arr::zstring)> f = 
    [start, stop](arr::zstring z) { return arr::zstring((string(z)).substr(start, stop)); };

  x->applyf(f);
  return x;
}



val::Value funcs::sys_time(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return val::make_array(std::chrono::system_clock::now());
}


// Function to obtain the default value of the increment.
template <typename F>
constexpr F getDefaultIncValue() { return 1; }

template <>
constexpr Global::duration getDefaultIncValue() { return Global::duration(1000000000); }


template<typename T>
void checkSeqArg(const T& arg, const yy::location& loc) {
  ;
}
template<>
void checkSeqArg(const double& arg, const yy::location& loc) {
  if (std::isinf(arg) || std::isnan(arg)) {
    throw interp::EvalException("argument cannot be 'NaN' or 'Inf'", loc);
  }
}


template<typename F, typename B, val::ValType F_T, val::ValType B_T>
static val::Value seq_helper_numeric(vector<val::VBuiltinG::arg_t>& v) {
  enum { FROM, TO, BY, LENGTH_OUT };

  auto to_t         = val::getVal(v[TO]).which();
  auto by_t         = val::getVal(v[BY]).which();
  auto length_out_t = val::getVal(v[LENGTH_OUT]).which();

  auto from = val::get_scalar<F>(val::getVal(v[FROM]));
  checkSeqArg<F>(from, val::getLoc(v[FROM]));
  // get out of the way all the invalid type situations:
  if (to_t != F_T && to_t != val::vt_null) 
    throw interp::EvalException("invalid type for argument", val::getLoc(v[TO]));
  if (length_out_t != val::vt_double && length_out_t != val::vt_null) 
    throw interp::EvalException("invalid type for argument", val::getLoc(v[LENGTH_OUT]));
  if (by_t != B_T && by_t != val::vt_null) 
    throw interp::EvalException("invalid type for argument", val::getLoc(v[BY]));

  B by = by_t != val::vt_null ? val::get_scalar<B>(val::getVal(v[BY])) : getDefaultIncValue<B>();
  checkSeqArg<B>(by, val::getLoc(v[BY]));

  // the logic itself:
  if (to_t != val::vt_null) {
    F to = val::get_scalar<F>(val::getVal(v[TO]));
    checkSeqArg<F>(to, val::getLoc(v[TO]));
    if (by_t == val::vt_null && to < from) {
      // if we are taking a default value for by, we need to follow
      // the direction set by 'from' and 'to':
      by *= -1;
    }
    if (length_out_t != val::vt_null) {
      auto length_out = funcs::getUint(val::get_scalar<double>(val::getVal(v[LENGTH_OUT])),
                                             val::getLoc(v[LENGTH_OUT]));
      by = (to - from) / (length_out - 1);
      checkSeqArg<B>(by, val::getLoc(v[BY]));
    }
    if (by == getInitValue<B>())
      throw interp::EvalException("'by' cannot be 0", val::getLoc(v[BY]));

    auto sameSign = ((to - from) >= getInitValue<B>() && by >= getInitValue<B>())
      || ((to - from) <  getInitValue<B>() && by <  getInitValue<B>());
                                 
    if (!sameSign)
      throw interp::EvalException("wrong sign in 'by' argument", val::getLoc(v[BY]));

    return arr::make_cow<arr::Array<F>>(false, arr::seq_to, from, to, to >= from ? by : -by);
  }
  else {                 // no 'to' argument
    if (length_out_t != val::vt_null) {
      auto length_out = funcs::getUint(val::get_scalar<double>(val::getVal(v[LENGTH_OUT])),
                                             val::getLoc(v[LENGTH_OUT]));
      return arr::make_cow<arr::Array<F>>(false, arr::seq_n, from, by, length_out);    
    }
    else
      throw interp::EvalException("missing argument", val::getLoc(v[TO]));
  }
}


template<typename T, val::ValType VT>
static val::Value seq_helper(vector<val::VBuiltinG::arg_t>& v) {
  enum { FROM, TO, BY, LENGTH_OUT, TZ };

  auto to_t         = val::getVal(v[TO]).which();
  auto length_out_t = val::getVal(v[LENGTH_OUT]).which();
  auto by_t         = val::getVal(v[BY]).which();
  auto tz_t         = val::getVal(v[TZ]).which();

  auto  from = val::get_scalar<T>(val::getVal(v[FROM]));
  // if 'to' is specified, then is must be a 'time'
  if (to_t != val::vt_null) {
    if (to_t != VT) {
      throw interp::EvalException("invalid type for argument", val::getLoc(v[TO]));
    }
  }

  // the two main cases hinge on whether 'by' is a period or not;
  // case 1: 'by' is a period:
  if (by_t == val::vt_period) {
    if (tz_t == val::vt_null) {
      throw interp::EvalException("argument 'tz' must be specified", val::getLoc(v[TZ]));
    }
    auto by = val::get_scalar<tz::period>(val::getVal(v[BY]));
    auto& tz = tzones.find(val::get_scalar<zstring>(val::getVal(v[TZ])));

    // figure out if it's 'to' or 'length.out' that is specified
    if (to_t != val::vt_null) {
      auto to = val::get_scalar<T>(val::getVal(v[TO]));
      return arr::make_cow<arr::Array<T>>(false, array_from_vector(ztsdb::seq(from, to, by, tz)));
    }
    else {                      // length.out was specified
      if (length_out_t == val::vt_null) {
        throw interp::EvalException("missing argument", val::getLoc(v[LENGTH_OUT]));
      }
      size_t length = val::get_scalar<double>(val::getVal(v[LENGTH_OUT]));
      return arr::make_cow<arr::Array<T>>(false, array_from_vector(ztsdb::seq(from, by, length, tz)));
    }
  }
  // case 2: 'by' is not a string, revert to numerical version:
  else {
    return seq_helper_numeric<T, Global::duration, VT, val::vt_duration>(v);
  }
}


val::Value funcs::seq(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { FROM, TO, BY, LENGTH_OUT, TZ };

  auto from_t       = val::getVal(v[FROM]).which();
  auto to_t         = val::getVal(v[TO]).which();
  auto length_out_t = val::getVal(v[LENGTH_OUT]).which();
  auto by_t         = val::getVal(v[BY]).which();

  if (to_t != val::vt_null && by_t != val::vt_null && length_out_t != val::vt_null) {
    throw interp::EvalException("too many arguments", val::getLoc(v[LENGTH_OUT]));
  }
  
  switch (from_t) {
  case val::vt_double: {
    return seq_helper_numeric<double, double, val::vt_double, val::vt_double>(v);
  }
  case val::vt_time: {
    return seq_helper<Global::dtime, val::vt_time>(v);
  }
  case val::vt_interval: {
    return seq_helper<tz::interval, val::vt_interval>(v);    
  }
  default:
    throw interp::EvalException("invalid type for argument", val::getLoc(v[FROM]));
  }
}


template<typename DS, typename DE>
static arr::zts align_wrapper(const arr::zts& ts, 
                              const Array<Global::dtime>& y, 
                              const DS& start, 
                              const DE& end,
                              const std::string& method,
                              const yy::location& methodloc) 
{
  // do the start/end and tz resolution here
  using FIter = Vector<double>::const_iterator;
  if (method == "closest") {
    return arr::align_closest<DS, DE>(ts, y, start, end);
  }
  if (method == "mean") {
    return arr::align_func<ztsdb::mean_element<FIter,double>, DS, DE>
      (ts, y, start, end);

  }
  if (method == "max") {
    return arr::align_func<ztsdb::max_element<FIter,double>, DS, DE>
      (ts, y, start, end);

  }
  if (method == "min") {
    return arr::align_func<ztsdb::min_element<FIter,double>, DS, DE>
      (ts, y, start, end);
  }
  if (method == "count") {
    return arr::align_func<ztsdb::count_element<FIter,double>, DS, DE>
      (ts, y, start, end);
  }
  if (method == "median") {
    return arr::align_func<ztsdb::median_element<FIter,double>, DS, DE>
      (ts, y, start, end);
  }

  throw interp::EvalException("unknown align method", methodloc);
}
  

template<typename SpU, typename SpV>
void checkVector(const SpU& u, const SpV& v, const yy::location& loc) {
  if (v->size() != 1 && (!v->isVector() || v->size() != u->size())) {
    throw interp::EvalException("invalid dimensions", loc);
  }
}


/// Align a zts onto a time vector.
val::Value funcs::align(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { FROM, TO, START, END, METHOD, TZ };

  auto start_t = val::getVal(v[START]).which();
  auto end_t = val::getVal(v[END]).which();
  const auto& to = get<val::SpVADT>(val::getVal(v[TO]));
  if (!to->isVector()) {
    throw interp::EvalException("'to' is not a vector", val::getLoc(v[TO]));    
  }
  const auto& method = val::get_scalar<arr::zstring>(val::getVal(v[METHOD]));
    
  const auto& from = get<val::SpZts>(val::getVal(v[FROM]));
  if (start_t == val::vt_duration && end_t == val::vt_duration) {
    const auto& start = get<val::SpVADUR>(val::getVal(v[START]));
    const auto& end   = get<val::SpVADUR>(val::getVal(v[END]));
    checkVector(to, start, val::getLoc(v[START]));
    checkVector(to, end,   val::getLoc(v[END]));
    return arr::make_cow<arr::zts>
      (false, 
       align_wrapper<arr::PseudoVector<Global::dtime, Global::duration>,
                     arr::PseudoVector<Global::dtime, Global::duration>>
       (*from, 
        *to, 
        arr::PseudoVector<Global::dtime, Global::duration>(start->getcol(0)), 
        arr::PseudoVector<Global::dtime, Global::duration>(end->getcol(0)), 
        method,
        val::getLoc(v[METHOD])));
  }
  else {
    if (val::getVal(v[TZ]).which() != val::vt_string) {
      throw interp::EvalException("time zone must be supplied", val::getLoc(v[TZ]));    
    }
    try {
      tzones.find(val::get_scalar<arr::zstring>(val::getVal(v[TZ])));
    }
    catch (...) {
      throw interp::EvalException("cannot find time zone", val::getLoc(v[TZ]));    
    }
    const auto& tz = tzones.find(val::get_scalar<arr::zstring>(val::getVal(v[TZ])));

    if (start_t == val::vt_period && end_t == val::vt_period) {
      const auto& start = get<val::SpVAPRD>(val::getVal(v[START]));
      const auto& end   = get<val::SpVAPRD>(val::getVal(v[END]));
      checkVector(to, start, val::getLoc(v[START]));
      checkVector(to, end,   val::getLoc(v[END]));
      return arr::make_cow<arr::zts>
        (false, align_wrapper<arr::PseudoVectorTz<Global::dtime, tz::period>,
                              arr::PseudoVectorTz<Global::dtime, tz::period>>
         (*from, 
          *to, 
          arr::PseudoVectorTz<Global::dtime, tz::period>(start->getcol(0), tz), 
          arr::PseudoVectorTz<Global::dtime, tz::period>(end->getcol(0), tz), 
          method,
          val::getLoc(v[METHOD])));
    }
    else if (start_t == val::vt_duration && end_t == val::vt_period) {
      const auto& start = get<val::SpVADUR>(val::getVal(v[START]));
      const auto& end   = get<val::SpVAPRD>(val::getVal(v[END]));
      checkVector(to, start, val::getLoc(v[START]));
      checkVector(to, end,   val::getLoc(v[END]));
      return arr::make_cow<arr::zts>
        (false, align_wrapper<arr::PseudoVector  <Global::dtime, Global::duration>,
                              arr::PseudoVectorTz<Global::dtime, tz::period>>
         (*from, 
          *to, 
          arr::PseudoVector<Global::dtime, Global::duration>(start->getcol(0)), 
          arr::PseudoVectorTz<Global::dtime, tz::period>(end->getcol(0), tz), 
          method,
          val::getLoc(v[METHOD])));
    }
    else {
      const auto& start = get<val::SpVAPRD>(val::getVal(v[START]));
      const auto& end   = get<val::SpVADUR>(val::getVal(v[END]));
      checkVector(to, start, val::getLoc(v[START]));
      checkVector(to, end,   val::getLoc(v[END]));
      return arr::make_cow<arr::zts>
        (false, align_wrapper<arr::PseudoVectorTz<Global::dtime, tz::period>, 
                              arr::PseudoVector  <Global::dtime, Global::duration>>
         (*from, 
          *to, 
          arr::PseudoVectorTz<Global::dtime, tz::period>(start->getcol(0), tz), 
          arr::PseudoVector<Global::dtime, Global::duration>(end->getcol(0)), 
          method,
          val::getLoc(v[METHOD])));
    }
  }
}


/// Find the indices of the alignment of a vector of time onto another.
val::Value funcs::align_idx(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { FROM, TO, START, END, TZ };

  auto start_t = val::getVal(v[START]).which();
  auto end_t   = val::getVal(v[END]).which();
  const auto& from = get<val::SpVADT>(val::getVal(v[FROM]));
  const auto& to = get<val::SpVADT>(val::getVal(v[TO]));
  if (!to->isVector()) {
    throw interp::EvalException("'to' is not a vector", val::getLoc(v[TO]));    
  }
  
  if (start_t == val::vt_duration && end_t == val::vt_duration) {
    const auto& start = get<val::SpVADUR>(val::getVal(v[START]));
    const auto& end   = get<val::SpVADUR>(val::getVal(v[END]));
    checkVector(to, start, val::getLoc(v[START]));
    checkVector(to, end,   val::getLoc(v[END]));
    auto idx = arr::align_idx<double, Global::NANF, 
                              arr::PseudoVector  <Global::dtime, Global::duration>,
                              arr::PseudoVector  <Global::dtime, Global::duration>>
      (from->getcol(0), 
       to->getcol(0), 
       arr::PseudoVector<Global::dtime, Global::duration>(start->getcol(0)), 
       arr::PseudoVector<Global::dtime, Global::duration>(end  ->getcol(0)));
    return arr::make_cow<val::VArrayD>(false, Vector<idx_type>{idx.size()}, idx);
  }
  else {
    if (val::getVal(v[TZ]).which() != val::vt_string) {
      throw interp::EvalException("time zone must be supplied", val::getLoc(v[TZ]));    
    }
    try {
      tzones.find(val::get_scalar<arr::zstring>(val::getVal(v[TZ])));
    }
    catch (...) {
      throw interp::EvalException("cannot find time zone", val::getLoc(v[TZ]));    
    }
    const auto& tz = tzones.find(val::get_scalar<arr::zstring>(val::getVal(v[TZ])));

    if (start_t == val::vt_period && end_t == val::vt_period) {
      auto start = get<val::SpVAPRD>(val::getVal(v[START]));
      auto end   = get<val::SpVAPRD>(val::getVal(v[END]));
      checkVector(to, start, val::getLoc(v[START]));
      checkVector(to, end,   val::getLoc(v[END]));
      auto idx = arr::align_idx<double, Global::NANF, 
                                arr::PseudoVectorTz<Global::dtime, tz::period>,
                                arr::PseudoVectorTz<Global::dtime, tz::period>> 
        (from->getcol(0), 
         to->getcol(0),
         arr::PseudoVectorTz<Global::dtime, tz::period>(start->getcol(0), tz),
         arr::PseudoVectorTz<Global::dtime, tz::period>(end->getcol(0), tz));
      return arr::make_cow<val::VArrayD>(false, Vector<idx_type>{idx.size()}, idx);
    }
    else if (start_t == val::vt_duration && end_t == val::vt_period) {
      auto start = get<val::SpVADUR>(val::getVal(v[START]));
      auto end   = get<val::SpVAPRD>(val::getVal(v[END]));
      checkVector(to, start, val::getLoc(v[START]));
      checkVector(to, end,   val::getLoc(v[END]));
      auto idx = arr::align_idx<double, Global::NANF, 
                                arr::PseudoVector<Global::dtime, Global::duration>,
                                arr::PseudoVectorTz<Global::dtime, tz::period>>
        (from->getcol(0), 
         to->getcol(0), 
         arr::PseudoVector<Global::dtime, Global::duration>(start->getcol(0)),
         arr::PseudoVectorTz<Global::dtime, tz::period>(end->getcol(0), tz));
      return arr::make_cow<val::VArrayD>(false, Vector<idx_type>{idx.size()}, idx);
    }
    else {
      const auto& start = get<val::SpVAPRD>(val::getVal(v[START]));
      const auto& end   = get<val::SpVADUR>(val::getVal(v[END]));
      checkVector(to, start, val::getLoc(v[START]));
      checkVector(to, end,   val::getLoc(v[END]));
      const auto idx = arr::align_idx<double, Global::NANF, 
                                      arr::PseudoVectorTz<Global::dtime, tz::period>,
                                      arr::PseudoVector<Global::dtime, Global::duration>>
        (from->getcol(0), 
         to->getcol(0), 
         arr::PseudoVectorTz<Global::dtime, tz::period>(start->getcol(0), tz),
         arr::PseudoVector  <Global::dtime, Global::duration>(end->getcol(0)));
      return arr::make_cow<val::VArrayD>(false, Vector<idx_type>{idx.size()}, idx);
    }
  }
}


val::Value funcs::dayweek(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { X, TZ };
  const auto& dt = get<val::SpVADT>(val::getVal(v[X]));
  auto& tz = tzones.find(val::get_scalar<zstring>(val::getVal(v[TZ])));

  return make_cow<arr::Array<double>>
    (false, arr::applyf<Global::dtime, double>
     (*dt, [&tz](Global::dtime u) { return static_cast<double>(ztsdb::dayweek(u,tz)); }));
}
val::Value funcs::daymonth(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { X, TZ };
  const auto& dt = get<val::SpVADT>(val::getVal(v[X]));
  auto& tz = tzones.find(val::get_scalar<zstring>(val::getVal(v[TZ])));

  return make_cow<arr::Array<double>>
    (false, arr::applyf<Global::dtime, double>
     (*dt, [&tz](Global::dtime u) { return static_cast<double>(ztsdb::daymonth(u,tz)); }));
  return val::VNull();
}
val::Value funcs::month(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { X, TZ };
  const auto& dt = get<val::SpVADT>(val::getVal(v[X]));
  auto& tz = tzones.find(val::get_scalar<zstring>(val::getVal(v[TZ])));

  return make_cow<arr::Array<double>>
    (false, arr::applyf<Global::dtime, double>
     (*dt, [&tz](Global::dtime u) { return static_cast<double>(ztsdb::month(u,tz)); }));
}
val::Value funcs::year(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { X, TZ };
  const auto& dt = get<val::SpVADT>(val::getVal(v[X]));
  auto& tz = tzones.find(val::get_scalar<zstring>(val::getVal(v[TZ])));

  return make_cow<arr::Array<double>>
    (false, arr::applyf<Global::dtime, double>
     (*dt, [&tz](Global::dtime u) { return static_cast<double>(ztsdb::year(u,tz)); }));
}


val::Value funcs::op_zts(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { X, Y, OP };
  const auto& x = get<val::SpZts>(val::getVal(v[X]));
  auto y = get<val::SpZts>(val::getVal(v[Y]));
  const auto& op = val::get_scalar<arr::zstring>(val::getVal(v[OP]));

  using FIter = Vector<double>::iterator;
  if (op == "*") {
    arr::op<ztsdb::applyd<FIter, ztsdb::multiplies<double,double,double>>>(*x, *y);
    return y;
  }
  if (op == "/") {
    arr::op<ztsdb::applyd<FIter, ztsdb::divides<double,double,double>>>(*x, *y);
    return y;
  }
  if (op == "+") {
    arr::op<ztsdb::applyd<FIter, ztsdb::plus<double,double,double>>>(*x, *y);
    return y;
  }
  if (op == "-") {
    arr::op<ztsdb::applyd<FIter, ztsdb::minus<double,double,double>>>(*x, *y);
    return y;
  }
  throw interp::EvalException("unknown align op", val::getLoc(v[OP]));
}
