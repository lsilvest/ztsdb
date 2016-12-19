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


#ifndef DOUBLE_HPP
#define DOUBLE_HPP


#include <string>
#include <sstream>
#include <double-conversion.h>


namespace ztsdb {
  
  /// Converts a double to a string representation. This is a quick
  /// function so that we can display doubles independently from the
  /// more complex Vector/Array display functions where the
  /// representation is dependent on the Vector set rather than just
  /// the individual double.
  inline std::string to_string(double d, int digits) {
    const int bufsz = 1024;
    char buf[bufsz];
    // see https://github.com/google/double-conversion
    double_conversion::DoubleToStringConverter dtosc(double_conversion::
                                                     DoubleToStringConverter::
                                                     EMIT_POSITIVE_EXPONENT_SIGN,
                                                     "Inf", "NaN", 'e', -6, 6, 6, 6);
    double_conversion::StringBuilder sb(buf, bufsz);
    dtosc.ToShortest(d, &sb);
    auto res = sb.position();
    sb.Finalize();
    if (res > digits) {
      double_conversion::StringBuilder sb2(buf, bufsz);
      dtosc.ToFixed(d, digits, &sb2);
    }

    std::stringstream ss;
    ss << buf;
    return ss.str();
  }

  inline std::string fixed_scientific(double d, int digits) {
    const int bufsz = 1024;
    char buf[bufsz];
    // see https://github.com/google/double-conversion
    double_conversion::DoubleToStringConverter dtosc(double_conversion::
                                                     DoubleToStringConverter::
                                                     EMIT_POSITIVE_EXPONENT_SIGN,
                                                     "Inf", "NaN", 'e', 0, 0, 0, 0);
    double_conversion::StringBuilder sb(buf, bufsz);
    dtosc.ToExponential(d, digits-1, &sb);
    sb.Finalize();

    return string(buf);
  }

  inline std::string fixed_decimal(double d, int digits) {
    const int bufsz = 1024;
    char buf[bufsz];
    // see https://github.com/google/double-conversion
    double_conversion::DoubleToStringConverter dtosc(double_conversion::
                                                     DoubleToStringConverter::
                                                     EMIT_POSITIVE_EXPONENT_SIGN,
                                                     "Inf", "NaN", 'e', 0, 0, 0, 0);
    double_conversion::StringBuilder sb(buf, bufsz);
    dtosc.ToFixed(d, digits-1, &sb);
    sb.Finalize();

    return string(buf);
  }


  inline std::string shortest_scientific(double d) {
    const int bufsz = 1024;
    char buf[bufsz];
    // see https://github.com/google/double-conversion
    double_conversion::DoubleToStringConverter dtosc(double_conversion::
                                                     DoubleToStringConverter::
                                                     EMIT_POSITIVE_EXPONENT_SIGN,
                                                     "Inf", "NaN", 'e', 0, 0, 0, 0);
    double_conversion::StringBuilder sb(buf, bufsz);
    dtosc.ToShortest(d, &sb);
    sb.Finalize();

    return string(buf);
  }

  inline std::string shortest_decimal(double d) {
    const int bufsz = 1024;
    char buf[bufsz];
    // see https://github.com/google/double-conversion
    double_conversion::DoubleToStringConverter dtosc(double_conversion::
                                                     DoubleToStringConverter::
                                                     EMIT_POSITIVE_EXPONENT_SIGN,
                                                     "Inf", "NaN", 'e', -99, 99, 0, 0);
    double_conversion::StringBuilder sb(buf, bufsz);
    dtosc.ToShortest(d, &sb);
    sb.Finalize();

    return string(buf);
  }

}


#endif
