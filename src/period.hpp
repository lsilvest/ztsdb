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

#ifndef PERIOD_HPP
#define PERIOD_HPP


#include <sstream>
#include "globals.hpp"
#include "timezone/zone.hpp"
#include "timezone/interval.hpp"


namespace tz {
  
  struct period {
    typedef int32_t month_t ;
    typedef int32_t day_t;

    period();
    period(int32_t months_p, int32_t days_p, Global::duration dur_p);
    period(const std::string& s);

    inline int32_t getMonths() const { return months; }
    inline int32_t getDays()   const { return days; }
    inline Global::duration getDuration() const { return dur; }
    inline void setMonths(int64_t m) { months = m;  }
    inline void setDays(int64_t d)   { days   = d;  }
    inline void setDuration(Global::duration d)    { dur = d;  }
    inline void addMonths(int64_t m)   { months += m; }
    inline void addDays(int64_t d)     { days   += d; }
    inline void addDuration(Global::duration d) { dur    += d; }

    inline bool operator==(const period& p) { return months==p.months && days==p.days; }
    inline bool operator!=(const period& p) { return months!=p.months || days!=p.days; }

  private:
    month_t months;
    day_t   days;
    Global::duration dur;
  };

  // move this block out so period doesn't have a dependency on
  // interval and timezone, or can we consider it's a natural
  // dependency? LLL
  Global::dtime plus (const Global::dtime& dt, const period& p,         const tz::Zone& z);
  Global::dtime plus (const period& p,         const Global::dtime& dt, const tz::Zone& z);
  Global::dtime minus(const Global::dtime& dt, const period& p,         const tz::Zone& z);
  tz::interval  plus (const tz::interval& i,   const period& p,         const tz::Zone& z);
  tz::interval  plus (const period& p,         const tz::interval& i,   const tz::Zone& z);
  tz::interval  minus(const tz::interval& i,   const period& p,         const tz::Zone& z);

  period operator-(const period& p);
  period operator+(const period& p1, const period& p2);
  period operator-(const period& p1, const period& p2);
  period operator*(const period& p1, double d);
  period operator*(double d,         const period& p1);
  period operator/(const period& p1, double d);

  bool operator==(const period& p1, const period& p2);
  bool operator!=(const period& p1, const period& p2);

  /// This operator is meaningless, but needs to be defined because of
  /// the intended template usage.
  inline bool operator<(const period& p1, const period& p2) { return false; }

  std::string to_string(const period& p);

}


#endif
