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


#include "conversion_funcs.hpp"
#include "base_types.hpp"
#include "display.hpp"



bool funcs::isTrue(val::Value x) {
  return val::get_scalar<bool>(funcs::value_convert<bool>(val::gval(x)));
}


static unsigned typeRank(val::ValType vt) {
  switch(vt) {
  case val::vt_null:
    return 0;
  case val::vt_bool:
    return 1;
  case val::vt_time:
  case val::vt_duration:
  case val::vt_period:
  case val::vt_interval:
    return 2;
  case val::vt_double:
    return 3;
  case val::vt_string:
    return 5;
  case val::vt_zts:
    return 6;
  case val::vt_list:
  case val::vt_connection:
  case val::vt_clos:
  case val::vt_builting:
  case val::vt_timer:
    return 7;
  default:
    throw std::range_error("typeRank: unknown type");
  }
}

bool funcs::operator<(val::ValType vt1, val::ValType vt2) {
  return typeRank(vt1) < typeRank(vt2);
}

bool funcs::operator>(val::ValType vt1, val::ValType vt2) {
  return typeRank(vt1) > typeRank(vt2);
}

bool funcs::operator==(val::ValType vt1, val::ValType vt2) {
  return typeRank(vt1) == typeRank(vt2);
}

// to array of double:
template<>
val::SpVAD funcs::array_convert(const val::SpVAD& u) {
  return u;
}
template<>
val::SpVAD funcs::array_convert(const val::SpVAB& u) {
  return arr::make_cow<val::VArrayD>(false, val::VArrayD(convert_cons, *u));
}
template<>
val::SpVAD funcs::array_convert(const val::SpVADUR& u) {
  return arr::make_cow<val::VArrayD>(false, val::VArrayD(convert_cons, *u));
}
template<>
val::SpVAD funcs::array_convert_from_scalar(const val::VNull& u) {
  return arr::make_cow<val::VArrayD>(false,
                                     Vector<arr::idx_type>{0}, 
                                     Vector<double>{});
}


// to array of bool:
template<>
val::SpVAB funcs::array_convert(const val::SpVAB& u) {
  return u;
}
template<>
val::SpVAB funcs::array_convert(const val::SpVAD& u) {
  return arr::make_cow<val::VArrayB>(false, val::VArrayB(convert_cons, *u));
}
template<>
val::SpVAB funcs::array_convert_from_scalar(const val::VNull& u) {
  return arr::make_cow<val::VArrayB>(false, 
                                     Vector<arr::idx_type>{0}, 
                                     Vector<bool>{});
}


// to array of duration:
template<>
val::SpVADUR funcs::array_convert(const val::SpVADUR& u) {
  return u;
}
template<>
val::SpVADUR funcs::array_convert(const val::SpVAD& u) {
  return arr::make_cow<val::VArrayDUR>(false, val::VArrayDUR(convert_cons, *u));
}
template<>
val::SpVADUR funcs::array_convert(const val::SpVAS& u) {
  return arr::make_cow<val::VArrayDUR>(false, val::VArrayDUR(convert_cons, *u));
}
template<>
val::SpVADUR funcs::array_convert_from_scalar(const val::VNull& u) {
  return arr::make_cow<val::VArrayDUR>(false,
                                       Vector<arr::idx_type>{0}, 
                                       Vector<Global::duration>{});
}


// to array of period:
template<>
val::SpVAPRD funcs::array_convert(const val::SpVAPRD& u) {
  return u;
}
template<>
val::SpVAPRD funcs::array_convert(const val::SpVAS& u) {
  return arr::make_cow<val::VArrayPRD>(false, val::VArrayPRD(convert_cons, *u));
}
template<>
val::SpVAPRD funcs::array_convert_from_scalar(const val::VNull& u) {
  return arr::make_cow<val::VArrayPRD>(false, 
                                       Vector<arr::idx_type>{0}, 
                                       Vector<tz::period>{});
}


// to array of time:
template<>
val::SpVADT funcs::array_convert(const val::SpVADT& u) {
  return u;
}
template<>
val::SpVADT funcs::array_convert(const val::SpVAS& u) {
  return arr::make_cow<val::VArrayDT>(false, val::VArrayDT(convert_cons, *u));
}
template<>
val::SpVADT funcs::array_convert_from_scalar(const val::VNull& u) {
  return arr::make_cow<val::VArrayDT>(false, 
                                      Vector<arr::idx_type>{0}, 
                                      Vector<Global::dtime>{});
}


// to array of interval:
template<>
val::SpVAIVL funcs::array_convert(const val::SpVAIVL& u) {
  return u;
}
template<>
val::SpVAIVL funcs::array_convert(const val::SpVAS& u) {
  return arr::make_cow<val::VArrayIVL>(false, val::VArrayIVL(convert_cons, *u));
}
template<>
val::SpVAIVL funcs::array_convert_from_scalar(const val::VNull& u) {
  return arr::make_cow<val::VArrayIVL>(false, 
                                       Vector<arr::idx_type>{0},
                                       Vector<tz::interval>{});
}


// to array of zstring:
template<>
val::SpVAS funcs::array_convert(const val::SpVAS& u) {
  return u;
}
template<>
val::SpVAS funcs::array_convert(const val::SpVAD& u) {
  return arr::make_cow<val::VArrayS>(false, val::VArrayS(convert_cons, *u));
}
template<>
val::SpVAS funcs::array_convert(const val::SpVAB& u) {
  return arr::make_cow<val::VArrayS>(false, val::VArrayS(convert_cons, *u));
}
template<>
val::SpVAS funcs::array_convert(const val::SpVADT& u) {
  return arr::make_cow<val::VArrayS>(false, val::VArrayS(convert_cons, *u));
}
template<>
val::SpVAS funcs::array_convert(const val::SpVADUR& u) {
  return arr::make_cow<val::VArrayS>(false, val::VArrayS(convert_cons, *u));
}
template<>
val::SpVAS funcs::array_convert(const val::SpVAIVL& u) {
  return arr::make_cow<val::VArrayS>(false, val::VArrayS(convert_cons, *u));
}
template<>
val::SpVAS funcs::array_convert(const val::SpVAPRD& u) {
  return arr::make_cow<val::VArrayS>(false, val::VArrayS(convert_cons, *u));
}
template<>
val::SpVAS funcs::array_convert_from_scalar(const val::VNull& u) {
  return arr::make_cow<val::VArrayS>(false, 
                                     Vector<arr::idx_type>{0},
                                     Vector<arr::zstring>{});
}
