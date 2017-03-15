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

  val::Value all(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value any(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);

  val::Value is_nan(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value is_infinite(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value all_equal(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);

  val::Value as_logical(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value as_integer(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value as_numeric(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value as_character(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value as_duration(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value as_period(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value as_time(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value as_interval(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value make_time(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value make_period(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value period_month(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value period_day(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value period_duration(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);

  val::Value character(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _interval(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value interval_start(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value interval_end(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value interval_sopen(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value interval_eopen(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);

  val::Value vlist(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value is_null(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);

  // array ------------> base_funcs_array.cpp
  val::Value c(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value t(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value length(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value make_connection(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value connection_port(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value connection_address(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value make_timer(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value make_zts(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value zts_idx(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value zts_data(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value zts_resize(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value zts_truncate(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value make_vector(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value make_matrix(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value make_array(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value ncol(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value nrow(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value dim(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value abind(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value rbind(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value cbind(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value subset(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value dblsubset(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value subassign(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value dblsubassign(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value get_typeof(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value load(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _sort(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _sort_idx(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value is_ordered(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value head(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value tail(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value runif(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value rnorm(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value set_seed(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value msync(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value alloc_dirname(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value lock(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value unlock(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value islocked(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  
  val::Value dyn_load(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _options(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);

  val::Value cat(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);  
  val::Value paste(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);  

  val::Value read_csv(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value substr(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value write_csv(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);

  val::Value dimnames(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value colnames(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value rownames(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);

  // the following need access to the continuation, so they are implemented in interp: 
  val::Value getvar(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value ls(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value assign(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value rm(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value source(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value tryCatch(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value do_call(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value print(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);  
  val::Value str(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);  

  val::Value stats_net(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value stats_msg(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value stats_ctx(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value info_net(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value info_msg(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 
  val::Value info_ctx(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic); 

  // math ------------> base_funcs_math.cpp
  val::Value _sin(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _cos(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _tan(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _sinh(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _cosh(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _tanh(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _asin(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _acos(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _atan(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _asinh(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _acosh(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _atanh(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _floor(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _ceiling(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value op(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  

  // rolling functions:
  val::Value rollmean(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value rollmin(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value rollmax(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value rollvar(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value rollcov(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value locf(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value move(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value rotate(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value diff(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value cumsum(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value cumprod(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value cumdiv(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value cummax(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value cummin(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value rev(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value sum(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value prod(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _min(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _max(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);

  // system and general utilities
  val::Value quit(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);  
  val::Value stop(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value system(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);

  // date/time
  val::Value sys_time(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);

  val::Value seq(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);

  // set functions
  val::Value intersect(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value _union(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value setdiff(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value intersect_idx(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value union_idx(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value setdiff_idx(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);

  val::Value align(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value align_idx(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value op_zts(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);

  // time
  val::Value dayweek(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value daymonth(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value month(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);
  val::Value year(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic);


  /// Replaces the boilerplate 'switch (<ValType>)'. Raises an exception on incorrect arg type.
  /// 1 arg:
  template <template<typename...> class F, typename... T>
  inline val::Value apply_to_types(val::Value& v, const yy::location& l) {
    throw interp::EvalException("incorrect argument type", l);
  }
  template <template<typename...> class F, val::ValType H, val::ValType... T>
  inline val::Value apply_to_types(val::Value& v, const yy::location& l) {
    return H == v.which() ? F<typename val::gettype<H>::TP>::f(v, l) : apply_to_types<F, T...>(v, l);
  }
  // 2 args:
  template <template<typename, typename FA1> class F, 
            typename FA1, typename... T>
  inline val::Value apply_to_types2(val::Value& v, FA1 a1, 
                                    const yy::location& lv, const yy::location& l1) {
    throw interp::EvalException("incorrect argument type", lv);
  }
  template <template<typename, typename FA1> class F, 
            typename FA1, val::ValType H, val::ValType... T>
  inline val::Value apply_to_types2(val::Value& v, FA1 a1, 
                                    const yy::location& lv, const yy::location& l1) {
    return H == v.which() ? F<typename val::gettype<H>::TP, FA1>::f(v, a1, lv, l1) : 
      apply_to_types2<F, FA1, T...>(v, a1, lv, l1);
  }
  // 3 args:
  template <template<typename, typename FA1, typename FA2> class F, 
            typename FA1, typename FA2, typename... T>
  inline val::Value apply_to_types3(val::Value& v, FA1 a1, FA2 a2, const yy::location& lv, 
                                    const yy::location& l1) {
    throw interp::EvalException("incorrect argument type", lv);
  }
  template <template<typename, typename FA1, typename FA2> class F, 
            typename FA1, typename FA2, val::ValType H, val::ValType... T>
  inline val::Value apply_to_types3(val::Value& v, FA1 a1, FA2 a2, 
                                    const yy::location& lv, const yy::location& l1) {
    return H == v.which() ? F<typename val::gettype<H>::TP, FA1, FA2>::f(v, a1, a2, lv, l1) : 
      apply_to_types3<F, FA1, FA2, T...>(v, a1, a2, lv, l1);
  }

  /// Replaces the boilerplate 'switch (<ValType>)'. Returns NULL on incorrect arg type.
  template <template<typename...> class F, typename... T>
  inline val::Value apply_to_types_null(val::Value& v, const yy::location& l) {
    return val::VNull();
  }
  template <template<typename...> class F, val::ValType H, val::ValType... T>
  inline val::Value apply_to_types_null(val::Value& v, const yy::location& l) {
    return H == v.which() ? F<typename val::gettype<H>::TP>::f(v) : apply_to_types_null<F, T...>(v, l);
  }

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
