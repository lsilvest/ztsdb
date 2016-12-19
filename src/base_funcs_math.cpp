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


extern tz::Zones tzones;


static inline val::Value mathfunc(const vector<val::VBuiltinG::arg_t>& v, 
                                  zcore::InterpCtx& ic,
                                  double(*f)(double)) {
  switch (getVal(v[0]).which()) {
  case val::vt_double: {
    auto a = get<val::SpVAD>(getVal(v[0]));
    a->applyf(f);               // will copy if not ref
    return a;
  }
  case val::vt_zts: {
    auto z = get<val::SpZts>(getVal(v[0]));
    z->applyf(f);               // will copy if not ref
    return z;
  }
  default:
    throw interp::EvalException("invalid type for argument", getLoc(v[0]));
  }
}

val::Value funcs::_sin(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return mathfunc(v, ic, std::sin);
}
val::Value funcs::_sinh(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return mathfunc(v, ic, std::sinh);
}
val::Value funcs::_cos(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return mathfunc(v, ic, std::cos);
}
val::Value funcs::_cosh(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return mathfunc(v, ic, std::cosh);
}
val::Value funcs::_tan(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return mathfunc(v, ic, std::tan);
}
val::Value funcs::_tanh(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return mathfunc(v, ic, std::tanh);
}
val::Value funcs::_asin(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return mathfunc(v, ic, std::asin);
}
val::Value funcs::_asinh(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return mathfunc(v, ic, std::asinh);
}
val::Value funcs::_acos(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return mathfunc(v, ic, std::acos);
}
val::Value funcs::_acosh(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return mathfunc(v, ic, std::acosh);
}
val::Value funcs::_atan(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return mathfunc(v, ic, std::atan);
}
val::Value funcs::_atanh(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return mathfunc(v, ic, std::atanh);
}


template <typename T, typename SPVA, T (*F)(T)>
static val::Value _floor_numeric_helper(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { X, UNIT, TZ };
  if (get<1>(v[UNIT]).which() != val::vt_null) {
    throw interp::EvalException("'unit' only meaningful for 'time' or 'interval'", get<2>(v[UNIT]));
  }
  auto a = get<SPVA>(get<1>(v[0]));
  a->applyf(F);
  return a;
}


template <typename T, typename SPVA, T (*F)(T, tz::Period), T (*FTZ)(T, tz::Period, const tz::Zone&)>
static val::Value _floor_dt_helper(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  using namespace std::placeholders;  // for _1, _2, _3...
  enum { X, UNIT, TZ };

  // we need a unit for dtime/interval ops:
  if (getVal(v[UNIT]).which() != val::vt_string) {
    throw interp::EvalException("invalid type for argument", getLoc(v[UNIT]));
  }
  auto s = val::get_scalar<arr::zstring>(get<1>(v[UNIT]));
  auto p = tz::unqualified_period_from_string(s);
  if (p >= tz::Period::DAY) {
    if (getVal(v[TZ]).which() == val::vt_null) {
      throw interp::EvalException("tz needed", getLoc(v[TZ]));
    }
    if (getVal(v[TZ]).which() != val::vt_string) {
      throw interp::EvalException("invalid type for argument", getLoc(v[TZ]));
    }
  }

  auto a = get<SPVA>(get<1>(v[0]));
  if (p >= tz::Period::DAY) {
    auto& z = tzones.find(val::get_scalar<arr::zstring>(getVal(v[TZ])));
    auto g = std::bind(FTZ, _1, p, std::ref(z));
    a->applyf(g);
  }
  else {
    auto g = std::bind(F, _1, p);
    a->applyf(g);
  }
  return a;
}


template <double (*FN)(double),
          template <typename T> class fdt, 
          template <typename T> class fdt_tz>
static val::Value _floor_helper(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  enum { X, UNIT, TZ };

  auto x_t = (get<1>(v[X]).which());

  switch (x_t) {
  case val::vt_double:
    return _floor_numeric_helper<double, val::SpVAD, FN>(v, ic);
  case val::vt_zts:
    return _floor_numeric_helper<double, val::SpZts, FN>(v, ic);
  case val::vt_time:
    return _floor_dt_helper
      <Global::dtime, val::SpVADT, fdt<Global::dtime>::f, fdt_tz<Global::dtime>::f>(v, ic);
  case val::vt_interval:
    return _floor_dt_helper
      <tz::interval, val::SpVAIVL, fdt<tz::interval>::f, fdt_tz<tz::interval>::f>(v, ic);
  default:
    throw interp::EvalException("invalid type for argument", get<2>(v[X]));
  }

}


// work around the fact we can't pass template template args to functions:
template <typename T>
struct Floor {
  static T f (T t, tz::Period p)  { return ztsdb::floor(t, p); }
};
template <typename T>
struct FloorTz {
  static T f (T t, tz::Period p, const tz::Zone& z) { return ztsdb::floor_tz(t, p, z); }
};

val::Value funcs::_floor(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return _floor_helper<std::floor, Floor, FloorTz>(v, ic);
}


// work around the fact we can't pass template template args to functions:
template <typename T>
struct Ceiling {
  static T f (T t, tz::Period p)  { return ztsdb::ceiling(t, p); }
};
template <typename T>
struct CeilingTz {
  static T f (T t, tz::Period p, const tz::Zone& z) { return ztsdb::ceiling_tz(t, p, z); }
};

val::Value funcs::_ceiling(const vector<val::VBuiltinG::arg_t>& v, zcore::InterpCtx& ic) {
  return _floor_helper<std::ceil, Ceiling, CeilingTz>(v, ic);
}
