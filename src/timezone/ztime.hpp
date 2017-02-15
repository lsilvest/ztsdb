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


#ifndef ZTIME_HPP
#define ZTIME_HPP


#include "../globals.hpp"
#include "../misc.hpp"
#include "zone.hpp"
#include "interval.hpp"


namespace tz {

  template <class Rep, class Period, class = std::enable_if_t<
            std::chrono::duration<Rep, Period>::min() < std::chrono::duration<Rep, Period>::zero()>>
    constexpr std::chrono::duration<Rep, Period> abs(std::chrono::duration<Rep, Period> d)
  {
    return d >= d.zero() ? d : -d;
  }

  Global::duration duration_from_string2(const std::string& s);


  std::string to_string(Global::dtime dt,
                        const std::string& format,
                        const tz::Zone& timezone,
                        const std::string& timezone_str,
                        bool abbrev=false,
                        bool fractional=false);

  Global::dtime dtime_from_numbers(int year, 
                                   unsigned month, 
                                   unsigned day, 
                                   unsigned hour,
                                   unsigned minute,
                                   unsigned second,
                                   unsigned nsecond,
                                   const tz::Zone& z);

  Global::dtime dtime_from_string(const std::string& s, 
                                  const tz::Zones& tzones,
                                  const std::string& fmt = "%Y-%m-%d %H:%M:%S[.%s] %Z",
                                  const std::string& tz = "");

  std::string to_string(const tz::interval& i,
                        const std::string& format,
                        const tz::Zone& timezone,
                        const std::string& timezone_str,
                        bool abbrev=false,
                        bool fractional=false);

  interval interval_from_string(const std::string& s, 
                                const tz::Zones& tzones,
                                const std::string& f = "%Y-%m-%d %H:%M:%S[.%s] %Z",
                                const std::string& tz = "");

  std::string to_string(Global::duration d);
  
  enum class Period : uint64_t { NANO, MICRO, MILLI, SECOND, MINUTE, HOUR, DAY, WEEK, MONTH, YEAR };

  tz::Period unqualified_period_from_string(const std::string& s);
}


namespace ztsdb {
  
  template<>
  struct multiplies<Global::duration, double, Global::duration> {
    inline Global::duration operator()(const Global::duration& t, const double& u) const {
      if (std::isnan(u)) {
        throw std::range_error("duration multiply by NaN");
      }
      else if (std::isinf(u)) {
        throw std::range_error("duration multiply by Inf");
      }
      return Global::duration(static_cast<Global::duration::rep>(t.count() * u));
    }
  };  

  template<>
  struct multiplies<double, Global::duration, Global::duration> {
    inline Global::duration operator()(const double& u, const Global::duration& t) const {
      if (std::isnan(u)) {
        throw std::range_error("duration multiply by NaN");
      }
      else if (std::isinf(u)) {
        throw std::range_error("duration multiply by Inf");
      }
      return Global::duration(static_cast<Global::duration::rep>(t.count() * u));
    }
  };  

  template<>
  struct divides<Global::duration, double, Global::duration> {
    inline Global::duration operator()(const Global::duration& t, const double& u) const {
      if (std::isnan(u)) {
        throw std::range_error("duration divide by NaN");
      } 
      else if (u == 0) {
        throw std::range_error("duration divide by 0");
      }
      else if (std::isinf(u)) {
        throw std::range_error("duration divide by Inf");
      }
      return Global::duration(static_cast<Global::duration::rep>(t.count() / u));
    }
  }; 

  Global::dtime floor(Global::dtime t, tz::Period p);
  tz::interval  floor(tz::interval i, tz::Period p);

  Global::dtime floor_tz(Global::dtime t, tz::Period p, const tz::Zone& z);
  tz::interval  floor_tz(tz::interval i, tz::Period p, const tz::Zone& z);

  Global::dtime ceiling(Global::dtime t, tz::Period p);
  tz::interval  ceiling(tz::interval i, tz::Period p);

  Global::dtime ceiling_tz(Global::dtime t, tz::Period p, const tz::Zone& z);
  tz::interval  ceiling_tz(tz::interval i, tz::Period p, const tz::Zone& z);

  size_t dayweek(Global::dtime, const tz::Zone& z);
  size_t daymonth(Global::dtime t, const tz::Zone& z);
  size_t month(Global::dtime t, const tz::Zone& z);
  ssize_t year(Global::dtime t, const tz::Zone& z);
  
}


#endif
