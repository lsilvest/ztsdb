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


#ifndef BASE_FUNCS
#define BASE_FUNCS

#include <vector>
#include <cmath>
#include <cfenv>
#include "valuevar.hpp"
#include "interp_error.hpp"


namespace zcore {
  struct InterpCtx;
}


// TODO:
// nchar
// mean
// rep
namespace funcs {

  val::Value all(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value any(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);

  val::Value is_nan(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value is_infinite(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value all_equal(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);

  val::Value as_logical(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value as_integer(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value as_numeric(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value as_character(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value as_duration(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value as_period(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value as_time(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value as_interval(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value make_time(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value make_period(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value period_month(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value period_day(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value period_duration(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);

  val::Value character(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _interval(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value interval_start(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value interval_end(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value interval_sopen(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value interval_eopen(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);

  val::Value vlist(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value is_null(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);

  // array ------------> base_funcs_array.cpp
  val::Value c(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value t(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value length(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value make_connection(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value connection_port(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value connection_address(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value make_timer(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value make_zts(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value zts_idx(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value zts_data(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value zts_resize(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value zts_truncate(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value make_vector(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value make_matrix(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value make_array(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value ncol(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value nrow(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value dim(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value abind(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value rbind(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value cbind(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value subset(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value dblsubset(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value subassign(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value dblsubassign(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value get_typeof(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value load(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _sort(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _sort_idx(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value is_ordered(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value head(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value tail(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value runif(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value rnorm(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value set_seed(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value msync(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value alloc_dirname(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);

  val::Value dyn_load(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _options(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);

  val::Value cat(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);  
  val::Value paste(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);  

  val::Value read_csv(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value substr(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value write_csv(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);

  val::Value dimnames(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value colnames(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value rownames(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);

  // the following need access to the continuation, so they are implemented in interp: 
  val::Value getvar(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value ls(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value assign(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value rm(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value source(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value tryCatch(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value do_call(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value print(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);  
  val::Value str(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);  

  val::Value stats_net(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value stats_msg(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value stats_ctx(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value info_net(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value info_msg(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value info_ctx(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 

  // math ------------> base_funcs_math.cpp
  val::Value _sin(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _cos(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _tan(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _sinh(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _cosh(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _tanh(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _asin(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _acos(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _atan(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _asinh(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _acosh(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _atanh(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _floor(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _ceiling(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);

  // rolling functions:
  val::Value rollmean(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value rollmin(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value rollmax(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value rollvar(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value rollcov(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value locf(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value move(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value rotate(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value diff(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value cumsum(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value cumprod(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value cumdiv(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value cummax(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value cummin(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value rev(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value sum(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value prod(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _min(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _max(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);

  // system and general utilities
  val::Value quit(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);  
  val::Value stop(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value system(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);

  // date/time
  val::Value sys_time(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);

  val::Value seq(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);

  // set functions
  val::Value intersect(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _union(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value setdiff(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value intersect_idx(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value union_idx(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value setdiff_idx(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);

  val::Value align(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value align_idx(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value op_zts(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);

  // time
  val::Value dayweek(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value daymonth(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value month(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value year(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);


  /// Replaces the boilerplate 'switch (<ValType>)'. Raises an exception on incorrect arg type.
  /// 1 arg:
  template <template<typename...> class F, typename... T>
  inline val::Value apply_to_types(val::Value v, const yy::location& l) {
    throw interp::EvalException("incorrect argument type", l);
  }
  template <template<typename...> class F, val::ValType H, val::ValType... T>
  inline val::Value apply_to_types(val::Value v, const yy::location& l) {
    return H == v.which() ? F<typename val::gettype<H>::TP>::f(v, l) : apply_to_types<F, T...>(v, l);
  }
  // 2 args:
  template <template<typename, typename FA1> class F, 
            typename FA1, typename... T>
  inline val::Value apply_to_types2(val::Value v, FA1 a1, 
                                    const yy::location& lv, const yy::location& l1) {
    throw interp::EvalException("incorrect argument type", lv);
  }
  template <template<typename, typename FA1> class F, 
            typename FA1, val::ValType H, val::ValType... T>
  inline val::Value apply_to_types2(val::Value v, FA1 a1, 
                                    const yy::location& lv, const yy::location& l1) {
    return H == v.which() ? F<typename val::gettype<H>::TP, FA1>::f(v, a1, lv, l1) : 
      apply_to_types2<F, FA1, T...>(v, a1, lv, l1);
  }
  // 3 args:
  template <template<typename, typename FA1, typename FA2> class F, 
            typename FA1, typename FA2, typename... T>
  inline val::Value apply_to_types3(val::Value v, FA1 a1, FA2 a2, const yy::location& lv, 
                                    const yy::location& l1) {
    throw interp::EvalException("incorrect argument type", lv);
  }
  template <template<typename, typename FA1, typename FA2> class F, 
            typename FA1, typename FA2, val::ValType H, val::ValType... T>
  inline val::Value apply_to_types3(val::Value v, FA1 a1, FA2 a2, 
                                    const yy::location& lv, const yy::location& l1) {
    return H == v.which() ? F<typename val::gettype<H>::TP, FA1, FA2>::f(v, a1, a2, lv, l1) : 
      apply_to_types3<F, FA1, FA2, T...>(v, a1, a2, lv, l1);
  }

  /// Replaces the boilerplate 'switch (<ValType>)'. Returns NULL on incorrect arg type.
  template <template<typename...> class F, typename... T>
  inline val::Value apply_to_types_null(val::Value v, const yy::location& l) {
    return val::VNull();
  }
  template <template<typename...> class F, val::ValType H, val::ValType... T>
  inline val::Value apply_to_types_null(val::Value v, const yy::location& l) {
    return H == v.which() ? F<typename val::gettype<H>::TP>::f(v) : apply_to_types_null<F, T...>(v, l);
  }

  // Makes accessing the tuples of 'val::VBuiltinG::arg_t' clearer.
#define getName(x) get<0>(x)
#define getVal(x)  get<1>(x) 
#define getLoc(x)  get<2>(x)

  /// Transforms a double into a an ssize_t, raising an exception if it's not possible.
  inline ssize_t getInt(double d, const yy::location& l) {
    std::feclearexcept(FE_ALL_EXCEPT);
    static_assert(sizeof(ssize_t)==sizeof(long long), "sizeof(ssize_t)==sizeof(long long)");
    auto res = llrint(d);
    if (std::fetestexcept(FE_INVALID)) {
      throw interp::EvalException("argument cannot be converted to an integer", l);
    }
    if (std::fetestexcept(FE_INEXACT)) {
      throw interp::EvalException("argument is not an integer", l);
    }
    return res;
  }

  /// Transforms a double into a a size_t, raising an exception if it's not possible.
  inline size_t getUint(double d, const yy::location& l) {
    if (d < 0) {
      throw interp::EvalException("argument cannot be negative", l);
    }
    return getInt(d, l);
  }
  
}

#endif
