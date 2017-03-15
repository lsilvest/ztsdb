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


#ifndef BASE_TYPES_HPP
#define BASE_TYPES_HPP


#include <cstdint>
#include <chrono>
#include <exception>
#include <string>
#include "globals.hpp"
#include "type_utils.hpp"
#include "timezone/interval.hpp"
#include "timezone/zone.hpp"
#include "timezone/ztime.hpp"


extern tz::Zones tzones;


namespace arr {

  // probably want to centralize type definition here instead of in Globals... LLL

  // using zdouble = double;
  // using zint    = int64_t;
  // using zbool   = bool;
  // using zstring = ZString<64>;
  // using dtime   = std::chrono::system_clock::time_point;

  template<>
  inline arr::zstring getInitValue() {
    return "";
  }
  template<>
  inline Global::dtime getInitValue() {
    return Global::dtime(Global::dtime::duration::zero());
  }
  template<>
  inline Global::duration getInitValue() {
    return Global::dtime::duration::zero();
  }
  template<>
  inline tz::interval getInitValue() {
    return tz::interval();
  }
  template<>
  inline tz::period getInitValue() {
    return tz::period();
  }

  /// the default case is that a conversion is not allowed.
  template<typename T, typename U>
  T convert(const U& u) {
    throw std::range_error(std::string("conversion not defined for ") + TypeName<U>::s +
                           " to " + TypeName<T>::s);
  }

  // because the template instantiation depends on the return value,
  // it cannot be automatically done by the compiler. So the calls to
  // convert are have full specification of the two types. This means
  // that we have to use function specialization rather than
  // overloading. So we need to be careful here.

  /// these are the set of "conversions" of a type to itself
  template<>
   inline double convert(const double& u) {
    return u;
  }
  template<>
  inline zstring convert(const zstring& u) {
    return u;
  }
  template<>
   inline bool convert(const bool& u) {
    return u;
  }
  template<>
   inline Global::dtime convert(const Global::dtime& u) {
    return u;
  }
  template<>
   inline Global::duration convert(const Global::duration& u) {
    return u;
  }
  template<>
  inline tz::period convert(const tz::period& u) {
    return u;
  }
  template<>
  inline tz::interval convert(const tz::interval& u) {
    return u;
  }
  
  /// conversions to zstring.
  template<>
  arr::zstring convert(const double& u);
  template<>
  arr::zstring convert(const bool& u);
  template<>
  arr::zstring convert(const Global::dtime& u);
  template<>
  arr::zstring convert(const Global::duration& u);
  template<>
  arr::zstring convert(const tz::period& u);
  template<>
  arr::zstring convert(const tz::interval& u);
  template<>
  inline arr::zstring convert(const std::string& u) {
    return u;
  }

  /// conversions to string.
  template<>
  std::string convert(const double& u);
  template<>
  std::string convert(const bool& u);
  template<>
  std::string convert(const Global::dtime& u);
  template<>
  std::string convert(const Global::duration& u);
  template<>
  std::string convert(const tz::period& u);
  template<>
  std::string convert(const tz::interval& u);
  template<>
  inline std::string convert(const std::string& u) {
    return u;
  }

  /// conversions to double.
  template<>
  inline double convert(const bool& u) {
    return u;
  }
  // for now don't propose this conversion:
  // template<>
  // inline double convert(const Global::dtime& u) {
  //   return u.time_since_epoch().count();
  // }
  template<>
  inline double convert(const Global::duration& u) {
    return u.count();
  }


  /// conversions to bool.
  template<>
  inline bool convert(const double& u) {
    return u;
  }
  template<>
  inline bool convert(const Global::duration& u) {
    return u.count();
  }


  /// conversions to duration.
  template<>
  inline Global::duration convert(const double& u) {
    return Global::duration(static_cast<const uint64_t>(u));
  }
  template<>
  inline Global::duration convert(const arr::zstring& u) {
    return tz::duration_from_string2(u);
  }
 
  /// conversions to period.
  template<>
  inline tz::period convert(const arr::zstring& u) {
    return tz::period(u);
  }
 
  /// conversions to time.
  template<>
  inline Global::dtime convert(const arr::zstring& u) {
    return tz::dtime_from_string(u, tzones);
  }

  /// conversions to interval.
  template<>
  inline tz::interval convert(const arr::zstring& u) {
    return tz::interval_from_string(u, tzones);
  }
}


#endif
