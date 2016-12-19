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


#include <regex>
#include "period.hpp"
#include "misc.hpp"
#include "timezone/ztime.hpp"
#include "hinnant_date/date.h"


using namespace std::chrono_literals;

tz::period::period() : months(0), days(0), dur(0s) { }

tz::period::period(int32_t months_p, int32_t days_p, Global::duration dur_p) : 
  months(months_p), days(days_p), dur(dur_p) { }


tz::period::period(const std::string& str) {
  using namespace std::string_literals;
  const char* s = str.c_str();
  const char* e = s + str.size();

  months = 0;
  days   = 0;
  dur    = 0s;

  int n;
  if (s < e && (*s == '/' || (s+2 < e && s[2] == ':'))) goto getduration;
  if (!readNumber(s, e, n, true) || s == e) throw std::range_error("cannot parse period");
  if (*s == 'y') {
    months += 12*n;
    ++s;
    if (s < e) {
      if (*s == '/') goto getduration;
      if (!readNumber(s, e, n, true) || s == e) throw std::range_error("cannot parse period");
    }
    else {
      return;
    }
  }      
  if (*s == 'm') {
    months += n;
    ++s;
    if (s < e) {
      if (*s == '/') goto getduration;
      if (!readNumber(s, e, n, true) || s == e) throw std::range_error("cannot parse period");      
    }
    else {
      return;
    }
  }      
  if (*s == 'w') {
    days += 7*n;
    ++s;
    if (s < e) {
      if (*s == '/') goto getduration;
      if (!readNumber(s, e, n, true) || s == e) throw std::range_error("cannot parse period");
    }
    else {
      return;
    }
  }
  if (*s == 'd') {
    days += n;
    ++s;
    if (s < e) { 
      if (*s == '/') goto getduration;
      if (!readNumber(s, e, n, true) || s == e) throw std::range_error("cannot parse period");
    }
    else {
      return;
    }
  }

  // we've succeeded a readNumber, so this means we've actually read
  // into the duration; so backtrack and use the already existing
  // function to parse a duration:
getduration:
  try {
    dur = tz::duration_from_string2(++s);
  }
  catch (...) {
    throw std::range_error("cannot parse period");
  }
}


std::string tz::to_string(const tz::period& p) {
  std::stringstream ss;
  ss << p.getMonths() << "m" << p.getDays() << "d/" << tz::to_string(p.getDuration());
  return ss.str();
}


Global::dtime tz::plus(const Global::dtime& dt, const tz::period& p, const tz::Zone& z) {
  int pos = -1;
  int direction = 1;            // unimportant, it's a fresh start
  auto offset = z.getoffset(dt, pos, direction);
  auto res = dt;
  if (p.getMonths()) {
    auto dt_floor = date::floor<date::days>(dt + offset);
    auto timeofday_offset = (dt + offset) - dt_floor;
    auto dt_ymd = date::year_month_day{dt_floor};
    dt_ymd += date::months(p.getMonths());
    res = date::sys_days(dt_ymd) - offset + timeofday_offset;
  }
  offset = z.getoffset(dt, pos, direction);
  res += p.getDays()*24h;
  res += p.getDuration();
  direction = res >= dt ? 1 : -1;
  auto newoffset = z.getoffset(res, pos, direction);
  if (newoffset != offset) {
    res += offset - newoffset; // adjust for DST or any other event that changed the TZ
  }
  return res;
}

Global::dtime tz::plus(const tz::period& p, const Global::dtime& dt, const tz::Zone& z) {
  return tz::plus(dt, p, z);
}

Global::dtime tz::minus(const Global::dtime& dt, const tz::period& p, const tz::Zone& z) {
  return tz::plus(dt, -p, z);
}


tz::interval tz::plus(const tz::interval& i, const tz::period& p, const tz::Zone& z) {
  return tz::interval(tz::plus(i.s, p, z), tz::plus(i.e, p, z), i.sopen, i.eopen);
}
  
tz::interval tz::plus(const tz::period& p, const tz::interval& i, const tz::Zone& z) {
  return tz::plus(i, p, z);
}

tz::interval tz::minus(const tz::interval& i, const tz::period& p, const tz::Zone& z) {
  return tz::plus(i, -p, z);
}


tz::period tz::operator+(const tz::period& p1, const tz::period& p2) {
  return tz::period(p1.getMonths()+p2.getMonths(), 
                    p1.getDays()+p2.getDays(), 
                    p1.getDuration()+p2.getDuration());
}

tz::period tz::operator-(const tz::period& p) {
  return tz::period(-p.getMonths(), -p.getDays(), -p.getDuration());
}

tz::period tz::operator-(const tz::period& p1, const tz::period& p2) {
  return tz::period(p1.getMonths()-p2.getMonths(), 
                    p1.getDays()-p2.getDays(),
                    p1.getDuration()-p2.getDuration());
}

tz::period tz::operator*(const tz::period& p, double d) {
  return tz::period(p.getMonths()*d, 
                    p.getDays()*d,
                    Global::duration(static_cast<int64_t>(d*p.getDuration().count())*
                                     Global::duration(1)));
}

tz::period tz::operator*(double d, const tz::period& p) {
  return p * d;
}

tz::period tz::operator/(const tz::period& p, double d) {
  if (d == 0) {
    throw std::logic_error("divide by zero");
  }
  return tz::period(p.getMonths()/d, 
                    p.getDays()/d,
                    Global::duration(static_cast<int64_t>(p.getDuration().count()/d)*
                                     Global::duration(1)));
}


// bool tz::operator>(const tz::period& p1, const tz::period& p2) {
//   // this is actually a difficult proposition, so for this calculation
//   // we take into account the average lengths. This means that in
//   // certain specific applications p1 might be <= to p2. But at any
//   // rate this should work for all practical purposes:
//   const auto YEAR = 365.25 * 24h;
//   const auto MONTH = YEAR/12;
//   return p1.getMonths()*MONTH + p1.getDays()*24h < p2.getMonths()*MONTH + p2.getDays()*24h;
// }

bool tz::operator==(const tz::period& p1, const tz::period& p2) {
  return 
    p1.getMonths() == p2.getMonths() && 
    p1.getDays() == p2.getDays() &&
    p1.getDuration() == p2.getDuration();
}

bool tz::operator!=(const tz::period& p1, const tz::period& p2) {
  return !(p1 == p2);
}
