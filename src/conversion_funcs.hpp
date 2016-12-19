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


#ifndef CONVERSION_FUNCS_HPP
#define CONVERSION_FUNCS_HPP

#include "valuevar.hpp"


namespace funcs {

  template<typename T, typename U>
  arr::cow_ptr<arr::Array<T>> array_convert(const arr::cow_ptr<arr::Array<U>>& u) {
    throw std::range_error(std::string("conversion not defined for array of ") + 
                           TypeName<U>::s + " to array of " + TypeName<T>::s);
  }

  template<typename T, typename U>
  arr::cow_ptr<arr::Array<T>> array_convert_from_scalar(const U& u) {
    throw std::range_error(std::string("conversion not defined for ") + 
                           TypeName<U>::s + " to array of " + TypeName<T>::s);
  }

  // to array of double:
  template<>
  val::SpVAD array_convert(const val::SpVAD& u);
  template<>
  val::SpVAD array_convert(const val::SpVAB& u);
  template<>
  val::SpVAD array_convert_from_scalar(const val::VNull& u);

  // to array of bool:
  template<>
  val::SpVAB array_convert(const val::SpVAB& u);
  template<>
  val::SpVAB array_convert(const val::SpVAD& u);
  template<>
  val::SpVAB array_convert_from_scalar(const val::VNull& u);

  // to array of duration:
  template<>
  val::SpVADUR array_convert(const val::SpVADUR& u);
  template<>
  val::SpVADUR array_convert(const val::SpVAD& u);
  template<>
  val::SpVADUR array_convert(const val::SpVAS& u);
  template<>
  val::SpVADUR array_convert_from_scalar(const val::VNull& u);

  // to array of period:
  template<>
  val::SpVAPRD array_convert(const val::SpVAPRD& u);
  template<>
  val::SpVAPRD array_convert(const val::SpVAS& u);
  template<>
  val::SpVAPRD array_convert_from_scalar(const val::VNull& u);

  // to array of time:
  template<>
  val::SpVADT array_convert(const val::SpVADT& u);
  template<>
  val::SpVADT array_convert(const val::SpVAS& u);
  template<>
  val::SpVADT array_convert_from_scalar(const val::VNull& u);

  // to array of interval:
  template<>
  val::SpVAIVL array_convert(const val::SpVAIVL& u);
  template<>
  val::SpVAIVL array_convert(const val::SpVAS& u);
  template<>
  val::SpVAIVL array_convert_from_scalar(const val::VNull& u);

  // to array of zstring:
  template<>
  val::SpVAS array_convert(const val::SpVAS& u);
  template<>
  val::SpVAS array_convert(const val::SpVAD& u);
  template<>
  val::SpVAS array_convert(const val::SpVAB& u);
  template<>
  val::SpVAS array_convert_from_scalar(const val::VNull& u);


  template<typename T>
  val::Value value_convert(const val::Value& x) {
    switch (x.which()) {
    case val::vt_null:
      return array_convert_from_scalar<T, val::VNull>(get<val::VNull>(x));
    case val::vt_double:
      return array_convert<T, double>(get<val::SpVAD>(x));
    case val::vt_bool:
      return array_convert<T, bool>(get<val::SpVAB>(x)); 
    case val::vt_duration:
      return array_convert<T, Global::duration>(get<val::SpVADUR>(x)); 
    case val::vt_period:
      return array_convert<T, tz::period>(get<val::SpVAPRD>(x)); 
    case val::vt_interval:
      return array_convert<T, tz::interval>(get<val::SpVAIVL>(x)); 
    case val::vt_string: {
      return array_convert<T, arr::zstring>(get<val::SpVAS>(x)); 
    }
    default:
      throw std::range_error(string("cannot convert to ") + TypeName<T>::s);
    }
  }

  // template<>
  // val::Value value_convert<arr::zstring>(val::Value x);

  // for the following functions, conversions of simple types are
  // defined in 'base_types.hpp' because they are needed by the type
  // 'Array<T>'. 'Array<T>' conversions are handled in the definition
  // file of this header.
  const auto convert_logical   = value_convert<bool>;
  const auto convert_numeric   = value_convert<double>;
  const auto convert_character = value_convert<arr::zstring>;
  const auto convert_duration  = value_convert<Global::duration>;  
  const auto convert_period    = value_convert<tz::period>;  
  const auto convert_time      = value_convert<Global::dtime>;  
  const auto convert_interval  = value_convert<tz::interval>;  

  bool operator<(val::ValType vt1, val::ValType vt2);
  bool operator>(val::ValType vt1, val::ValType vt2);
  bool operator==(val::ValType vt1, val::ValType vt2);

  bool isTrue(val::Value x);

}





#endif
