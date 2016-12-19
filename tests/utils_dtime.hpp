// Copyright (C) 2015 Leonardo Silvestri
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

#ifndef TEST_UTILS_DTIME_HPP
#define TEST_UTILS_DTIME_HPP


#include "globals.hpp"
#include "timezone/ztime.hpp"


using namespace std::literals;


const double TNAN = std::numeric_limits<double>::quiet_NaN();
template <typename I>
struct NANF {
};
template<>
struct NANF<double> {
  static double f() { return TNAN; }
};


// ad-hoc helper functions to make it easy to create a time:
inline Global::dtime mkt(unsigned t) {
  if (t > 99) {
    throw std::out_of_range("mkt: t must be < 99");
  }
  std::string tt = t < 10 ? "0"s + std::to_string(t) : std::to_string(t);
  return tz::dtime_from_string("2015-03-09 06:38:00."s + tt + " America/New_York"s);
}


// ad-hoc helper functions to make it easy to create an interval:
inline tz::interval mki(unsigned s, unsigned e, bool sopen=false, bool eopen=false) {
  if (s > 99 || e > 99) {
    throw std::out_of_range("mki: s and e must be < 99");
  }
  std::string ss = s < 10 ? "0"s + std::to_string(s) : std::to_string(s);
  std::string se = e < 10 ? "0"s + std::to_string(e) : std::to_string(e);
  std::string ssopen = sopen ? "-" : "+";
  std::string seopen = eopen ? "-" : "+";
                       
  return tz::interval_from_string
                       ("|"s + ssopen + "2015-03-09 06:38:00."s + ss + " America/New_York "s
                        "-> 2015-03-09 06:38:00."s + se + " America/New_York"s + seopen + "|"s);
}

#endif
