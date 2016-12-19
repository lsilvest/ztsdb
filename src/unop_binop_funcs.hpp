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


#ifndef UNOP_BINOP_FUNCS
#define UNOP_BINOP_FUNCS

#include <string>
#include "valuevar.hpp"


namespace funcs {

  /// This corrects the problem that pow(1, NaN) in stdlib returns 1.
  constexpr double pow(double d1, double d2) { return std::isnan(d2) ? d2 : std::pow(d1, d2); }

  template <typename T>
  struct power : public binary_function<T, T, T> {
    constexpr T operator()(const T& x, const T& y) const { return pow(x, y); }
  };

  /// Calculate not equal using negation of equality.
  template <typename T>
  struct not_equal_to : public binary_function<T, T, T> {
    constexpr bool operator()(const T& x, const T& y) const { return !(x == y); }
  };

  val::Value evalunop(val::Value v, int op);
  val::Value evalbinop(val::Value v1, const val::Value& v2, int op, const val::Value& attrib);

}

#endif
