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


#include <cmath>
#include "interp_ctx.hpp"
#include "base_funcs.hpp"
#include "timezone/ztime.hpp"
#include "timezone/vector_set_time.hpp"
#include "vector_set.hpp"
#include "valuevar.hpp"


extern tz::Zones tzones;


// -----------------------------------------------------------------------------------------
// set functions that return the set (later we have the set functions that return indices):


/// Replaces the boilerplate 'switch (<ValType>)'. Raises an exception on incorrect arg type.
template <template<typename, typename> class F, typename... T>
static inline val::Value apply_to_types_xy(val::Value x, val::Value y,
                                           const yy::location& lx, const yy::location& ly) {
  throw interp::EvalException("incorrect argument type", lx);
}
template <template<typename, typename> class F, val::ValType H, val::ValType L, val::ValType... T>
static inline val::Value apply_to_types_xy(val::Value x, val::Value y,
                                           const yy::location& lx, const yy::location& ly) {
  return H == x.which() && L == y.which() ? F<typename val::getelttype<H>::TP,
                                              typename val::getelttype<L>::TP>::f(x, y, lx, ly)
    : apply_to_types_xy<F, T...>(x, y, lx, ly);
}


template<typename T, typename U, template<typename,typename> class F>
struct set_wrapper {
  static val::Value f(val::Value& x, val::Value& y, const yy::location& lx, const yy::location& ly) {
    auto xx = get<arr::cow_ptr<arr::Array<T>>>(x);
    arr::cow_ptr<arr::Array<U>> yy(false, nullptr);
    try {
      yy = get<arr::cow_ptr<arr::Array<U>>>(y);
    }
    catch (...) {
      throw interp::EvalException("incorrect argument type", ly);
    }
    if (!xx->isVector()) {
      throw interp::EvalException("argument must be a vector", lx);
    }
    if (!yy->isVector()) {
      throw interp::EvalException("argument must be a vector", ly);
    }
    auto res = F<T,U>::f(*(xx->v[0].get()), *(yy->v[0].get()));
    return arr::make_cow<arr::Array<T>>(false, std::move(res));
  }
};


// intersect --------------------
template<typename T, typename U>
struct intersect_func_wrapper {
  static Vector<T> f(const Vector<T>& v1, const Vector<U>& v2) { return intersect(v1, v2); }
};
template <typename T, typename U>
using intersect_wrapper = set_wrapper<T, U, intersect_func_wrapper>;

val::Value funcs::intersect(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic)
{
  enum { X, Y };
  return apply_to_types_xy<intersect_wrapper, 
                           val::vt_double,   val::vt_double,
                           val::vt_bool,     val::vt_bool,
                           val::vt_time,     val::vt_time, 
                           val::vt_duration, val::vt_duration, 
                           val::vt_interval, val::vt_interval, 
                           val::vt_string,   val::vt_string,   
                           val::vt_time,     val::vt_time,
                           val::vt_time,     val::vt_interval
                           >(val::getVal(v[X]), val::getVal(v[Y]), val::getLoc(v[X]), val::getLoc(v[Y]));
}


// union ----------------------
template<typename T, typename U>
struct union_func_wrapper {
  static Vector<T> f(const Vector<T>& v1, const Vector<U>& v2) { return _union(v1, v2); }
};
template <typename T, typename U>
using union_wrapper = set_wrapper<T, U, union_func_wrapper>;

val::Value funcs::_union(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { X, Y };
  return apply_to_types_xy<union_wrapper, 
                           val::vt_double,   val::vt_double,
                           val::vt_bool,     val::vt_bool,
                           val::vt_time,     val::vt_time, 
                           val::vt_duration, val::vt_duration, 
                           val::vt_interval, val::vt_interval, 
                           val::vt_string,   val::vt_string,   
                           val::vt_time,     val::vt_time
                           >(val::getVal(v[X]), val::getVal(v[Y]), val::getLoc(v[X]), val::getLoc(v[Y]));

}

// setdiff --------------------
template<typename T, typename U>
struct setdiff_func_wrapper {
  static Vector<T> f(const Vector<T>& v1, const Vector<U>& v2) { return setdiff(v1, v2); }
};
template <typename T, typename U>
using setdiff_wrapper = set_wrapper<T, U, setdiff_func_wrapper>;

val::Value funcs::setdiff(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { X, Y };
  return apply_to_types_xy<setdiff_wrapper, 
                           val::vt_double,   val::vt_double,
                           val::vt_bool,     val::vt_bool,
                           val::vt_time,     val::vt_time, 
                           val::vt_duration, val::vt_duration, 
                           val::vt_interval, val::vt_interval, 
                           val::vt_string,   val::vt_string,   
                           val::vt_time,     val::vt_time,
                           val::vt_time,     val::vt_interval
                           >(val::getVal(v[X]), val::getVal(v[Y]), val::getLoc(v[X]), val::getLoc(v[Y]));
}



// -----------------------------------------------------------------------------------------
// set functions that return indices:


/// Fairly similar to its non-idx counterpart, except that we return a list of idx vectors:
template<typename T, typename U, template<typename,typename> class F>
struct set_wrapper_idx {
  static val::Value f(val::Value& x, val::Value& y, const yy::location& lx, const yy::location& ly) {
    auto xx = get<arr::cow_ptr<arr::Array<T>>>(x);
    arr::cow_ptr<arr::Array<U>> yy(false, nullptr);
    try {
      yy = get<arr::cow_ptr<arr::Array<U>>>(y);
    }
    catch (...) {
      throw interp::EvalException("incorrect argument type", ly);
    }
    if (!xx->isVector()) {
      throw interp::EvalException("argument must be a vector", lx);
    }
    if (!yy->isVector()) {
      throw interp::EvalException("argument must be a vector", ly);
    }

    auto res = F<T,U>::f(*(xx->v[0].get()), *(yy->v[0].get()));
    auto val1 = val::Value(arr::make_cow<val::VArrayD>(false, std::move(res.first)));
    auto val2 = val::Value(arr::make_cow<val::VArrayD>(false, std::move(res.second)));
    
    return arr::make_cow<val::VList>(false, Array<val::Value>(Vector<val::Value>{val1, val2}));
  }
};

// intersect_idx --------------------
template<typename T, typename U>
struct intersect_idx_func_wrapper {
  static std::pair<Vector<double>, Vector<double>>
    f(const Vector<T>& v1, const Vector<U>& v2) { return intersect_idx<T, U, double>(v1, v2); }
};
template <typename T, typename U>
using intersect_idx_wrapper = set_wrapper_idx<T, U, intersect_idx_func_wrapper>;

val::Value funcs::intersect_idx(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic)
{
  enum { X, Y };
  return apply_to_types_xy<intersect_idx_wrapper, 
                           val::vt_double,   val::vt_double,
                           val::vt_bool,     val::vt_bool,
                           val::vt_time,     val::vt_time, 
                           val::vt_duration, val::vt_duration, 
                           val::vt_interval, val::vt_interval, 
                           val::vt_string,   val::vt_string,   
                           val::vt_time,     val::vt_time,
                           val::vt_time,     val::vt_interval
                           >(val::getVal(v[X]), val::getVal(v[Y]), val::getLoc(v[X]), val::getLoc(v[Y]));
}

// union_idx --------------------

/// The indices from a union_idx have NaN where an element was not
/// selected for the union, and the function template requires a
/// function capable of producing a NaN for the result template type
/// (in our case 'double').
template <typename I>
struct NANF {
};
template<>
struct NANF<double> {
  static double f() { return Global::ZNAN; }
};

template<typename T, typename U>
struct union_idx_func_wrapper {
  static std::pair<Vector<double>, Vector<double>>
    f(const Vector<T>& v1, const Vector<U>& v2) { return union_idx<T, U, double, NANF>(v1, v2); }
};
template <typename T, typename U>
using union_idx_wrapper = set_wrapper_idx<T, U, union_idx_func_wrapper>;

val::Value funcs::union_idx(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic)
{
  enum { X, Y };
  return apply_to_types_xy<union_idx_wrapper, 
                           val::vt_double,   val::vt_double,
                           val::vt_bool,     val::vt_bool,
                           val::vt_time,     val::vt_time, 
                           val::vt_duration, val::vt_duration, 
                           val::vt_interval, val::vt_interval, 
                           val::vt_string,   val::vt_string,   
                           val::vt_time,     val::vt_time
                           >(val::getVal(v[X]), val::getVal(v[Y]), val::getLoc(v[X]), val::getLoc(v[Y]));
}

// setdiff_idx --------------------
template<typename T, typename U>
struct setdiff_idx_func_wrapper {
  static std::pair<Vector<double>, Vector<double>>
    f(const Vector<T>& v1, const Vector<U>& v2) { return setdiff_idx<T, U, double>(v1, v2); }
};
template <typename T, typename U>
using setdiff_idx_wrapper = set_wrapper_idx<T, U, setdiff_idx_func_wrapper>;

val::Value funcs::setdiff_idx(vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic)
{
  enum { X, Y };
  return apply_to_types_xy<setdiff_idx_wrapper, 
                           val::vt_double,   val::vt_double,
                           val::vt_bool,     val::vt_bool,
                           val::vt_time,     val::vt_time, 
                           val::vt_duration, val::vt_duration, 
                           val::vt_interval, val::vt_interval, 
                           val::vt_string,   val::vt_string,   
                           val::vt_time,     val::vt_time,
                           val::vt_time,     val::vt_interval
                           >(val::getVal(v[X]), val::getVal(v[Y]), val::getLoc(v[X]), val::getLoc(v[Y]));
}
