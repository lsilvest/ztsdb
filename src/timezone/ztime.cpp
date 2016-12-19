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


#include <iostream>
#include <sstream>
#include <iomanip>
#include <regex>
#include <stdexcept>
#include <tuple>
#include <cctype>
#include "ztime.hpp"
#include "../hinnant_date/date.h"


extern tz::Zones tzones;
using namespace std::chrono_literals;



static Global::duration average_duration(tz::Period p) {
  using namespace std::chrono;
  using namespace std::literals;
  switch (p) {
  case tz::Period::NANO:   return 1ns;
  case tz::Period::MICRO:  return 1us;
  case tz::Period::MILLI:  return 1ms;
  case tz::Period::SECOND: return 1s;
  case tz::Period::MINUTE: return 1min;
  case tz::Period::HOUR:   return 1h;
  case tz::Period::DAY:    return 24h;
  case tz::Period::WEEK:   return 7*24h;
  case tz::Period::MONTH:  return 31*24h;
  case tz::Period::YEAR:   return 365*24h;
  default:
    throw std::range_error("duration_from_period: unknown period");
  }
}
    

Global::duration tz::duration_from_string2(const std::string& str) {
  Global::duration d = 0s;
  const char* s = str.c_str();
  const char* e = s + str.size();

  auto sign = 1;
  if (s < e && *s == '-') {
    sign = -1;
    ++s;
  }

  int n;
  if (!readNumber(s, e, n, false)) throw std::range_error("cannot parse duration");

  if (s < e && *s == ':') {
    // we've got HHH:MM:SS format
    d += n * 1h;
    ++s;
    if (s + 5 > e || !isdigit(s[0]) || !isdigit(s[1]) || 
        s[2] != ':' || !isdigit(s[3]) || !isdigit(s[4])) {
      throw std::range_error("cannot parse duration");
    }
    d += ((s[0] - '0')*10 + (s[1] - '0'))*1min;

    // treat seconds in the general way:
    n = (s[3] - '0')*10 + (s[4] - '0');
    s += 5;
  }

  d += n * 1s;
  if (s == e) return sign*d;

  if (*s++ != '.') throw std::range_error("cannot parse duration");
  Global::duration mul = 100ms;
  unsigned i = 0;
  while (s < e) {
    if (mul < 1ns) throw std::range_error("cannot parse duration");
    if ((i == 3 || i == 6) && *s == '_') { ++s; continue; }
    ++i;
    if (!isdigit(*s)) throw std::range_error("cannot parse duration");
    d += (*s - '0') * mul;
    mul /= 10;
    ++s;
  }

  if (s == e) 
    return sign*d;
  else
    throw std::range_error("cannot parse duration");
}


std::string tz::to_string(Global::dtime dt,
                          const std::string& format,
                          const std::string& timezone,
                          bool abbrev,
                          bool fractional) {
  // look up the tz and find the tz offset:
  const auto& tzone = tzones.find(timezone);
  auto offset_abbrev = tzone.getoffset(dt);
  dt += offset_abbrev.first;

  date::sys_days t_days = date::floor<date::days>(dt);
  auto ymd = date::year_month_day(t_days);
  
  static_assert(Global::dtime::duration::period::num == 1, "period numerator must be 1");
  const int64_t den = Global::dtime::duration::period::den;
  int64_t zd = dt.time_since_epoch().count() / den;
  int64_t nsec = dt.time_since_epoch().count() - zd * den;
  if (nsec < 0) {
    nsec += den;
    --zd;
  }
  int64_t z = floor(static_cast<double>(zd) / (3600 * 24));
  int64_t zrem = zd - z * (3600 * 24);

  const unsigned h = zrem / 3600;
  const unsigned hrem = zrem % 3600;
  const unsigned mn = hrem / 60;
  const unsigned s = hrem % 60;

  std::stringstream ss;
  ss << std::setfill ('0') 
     << std::setw(4) << ymd.year()  << '-' 
     << std::setw(2) << unsigned(ymd.month()) << '-' 
     << std::setw(2) << ymd.day()   << ' ' 
     << std::setw(2) << h  << ':' 
     << std::setw(2) << mn << ':' 
     << std::setw(2) << s;
  if (nsec || fractional) {
    ss << '.' << std::setw(9) << nsec;
  }
  if (abbrev) {
    ss << ' ' << offset_abbrev.second;
  }
  else {
    ss << ' ' << timezone;
  }
  return ss.str();
}


static inline void skipWhitespace(const char*& sp, const char* const se) {
  while (sp < se) {
    if (*sp == ' ' || *sp == '\t') {
      ++sp;
    }
    else {
      break;
    }
  }
}


static inline void skipOptional(const char*& sp, const char* const se) {
  while (sp < se) {
    if (*sp++ == ']') {
      break;
    }
  }
}


/// Read an integer. This functions does not read beyond the end of
/// 'sp' (i.e. 'se') and does not read more that expectmax
/// characters. If the number of characters read is smaller than
/// 'expectmin' the function raises an exception.
static inline size_t readInt(const char*& sp, 
                             const char* const se,
                             const int expectmin,
                             const int expectmax,
                             int padding = 0) {
  const char* s = sp;
  int res = 0;
  int i;
  for (i=0; i<expectmax && sp < se; ++i) {
    if (*sp >= '0' && *sp <= '9') {
      res = 10 * res + *sp++ - 0x30;
    }
    else {
      break;
    }
  }
  if (padding) {
    res *= pow(10, padding - i);
  }
  if (sp - s >= expectmin) {
    return res;
  }
  else {
    throw std::range_error("couldn't parse datetime element");
  }
}


static inline const std::string readString(const char*& sp, const char* const se) {
  const char* const s = sp;
  while (sp < se) {
    if ((*sp >= 'A' && *sp <= 'Z') || 
        (*sp >= 'a' && *sp <= 'z') || 
        (*sp == '_')               || 
        (*sp >= '/' && *sp <= '9')) {
      ++sp;
    }
    else {
      break;
    }
  }  
  if (sp > s) {
    return std::string(s, sp-s);
  } 
  else {
    throw std::range_error("couldn't parse datetime timezone");
  }
}


using timetuple = std::tuple<int, unsigned, unsigned, unsigned, 
                             unsigned, unsigned, unsigned, std::string>;

static inline timetuple readDtime(const char*& sp, const char* const se, 
                                      const char*& fp, const char* const fe) 
{
  auto y = 0, m = 0, d = 0, h = 0, mn = 0, sec = 0, nsec = 0;
  std::string tz = "";
  bool optional = false;

  while (sp < se && fp < fe) {
    if (sp < se && *fp == '[') {
      ++fp;
      optional = true;
    }
    else if (sp < se  && *fp == ']') {
      ++fp;
      optional = false;
    }
    else {
      if (*fp == '%') {
        if (++fp < fe) {
          switch (*fp) {
          case 'm': m    = readInt(sp, se, 1, 2);           break;
          case 'd': d    = readInt(sp, se, 1, 2);           break;
          case 'y': y    = 2000 + readInt(sp, se, 2, 2);    break;
          case 'Y': y    = readInt(sp, se, 4, 4);           break;  
          case 'H': h    = readInt(sp, se, 1, 2);           break;
          case 'M': mn   = readInt(sp, se, 1, 2);           break;
          case 'S': sec  = readInt(sp, se, 1, 2);           break;
            // the padding is dependent on the precision of our duration type, so 
            // we need to enter something better than 9 below LLL
          case 's': nsec = readInt(sp, se, 0, 9, 9);        break;
          case 'Z': tz   = readString(sp, se);        break;
          default:
            throw std::range_error(std::string("unknown datetime format character '") + *fp + '\'');
          }
          ++fp;                 // move after the format character
        }
        else {
          throw std::range_error("missing format control character");
        }
      }
      else {                    // match char
        if (*sp == *fp) {
          ++sp;
          ++fp;
        }
        else {
          if (!optional) {
            throw std::range_error("invalid date format");
          }
          else {
            optional = false;
            skipOptional(fp, fe);
          }
        }
      }
    }
  }

  return timetuple(y, m, d, h, mn, sec, nsec, tz);
}


Global::dtime tz::dtime_from_numbers(int y, 
                                     unsigned m, 
                                     unsigned d, 
                                     unsigned h,
                                     unsigned mn,
                                     unsigned sec,
                                     unsigned nsec,
                                     const tz::Zone& tz) {
  // not as much as we could test, but good enough without too much of
  // a performance hit:
  if (m < 1 || m > 12) {
    throw std::range_error("month must be >= 1 and <= 12");
  }
  if (d < 1 || d > 31) {
    throw std::range_error("day must be >= 1 and <= 31");    
  }
  if (h > 23) {
    throw std::range_error("hour must be < 24");
  }
  if (mn > 59) {
    throw std::range_error("minute must be < 60");
  }
  if (sec > 59) {
    throw std::range_error("second must be < 60"); // debatable
  }
  if (nsec > 999999999) {
    throw std::range_error("second must be < 1000000000");
  }

  using namespace std::chrono;
  using namespace std::literals;
  Global::dtime dt = 
    date::sys_days(date::year_month_day(date::year(y), date::month(m), date::day(d))) +
    (static_cast<int>(h) * 3600 + static_cast<int>(mn * 60) + static_cast<int>(sec)) * 1s + 
    static_cast<int>(nsec) * 1ns;

  // won't work, see abbrev comment in zone.cpp LLL
  // // find the tz offset; it can be either of two possibilities: 

  // // 1. An abbreviation, in which case we can immediately get the
  // // offset (and note that we trust that the time is well formed -
  // // users wanting to be cautious would use the full timezone name):
  // try {
  //   auto offset = tzones.getoffset(tz);
  //   // std::cout << "offset found: " << offset.count() << std::endl;
  //   return dt - offset;
  // }
  // catch (...) {
  //   // continue
  // }

  // 2. A full timezone name (we look here if we didn't find the
  // abbreviation), in which case we look for the proper offset at
  // that time and we need to handle both ambiguous or impossible
  // dates:
  auto rvoset = tz.getReverseOffset(dt);
  // make error message more meaningful LLL
  if (rvoset.size() == 0) {
    throw std::range_error("datetime is not representable");
  } 
  if (rvoset.size() == 2) {
    throw std::range_error("datetime is ambiguous");
  }
  
  return dt - *rvoset.begin(); 
}


static Global::dtime dtime_from_timetuple(const timetuple& tpl) {
  enum { Y, M, D, H, MN, SEC, NSEC, TZ };

  if (std::get<TZ>(tpl) == "") {
    throw std::range_error("timezone must be specified");
  }
  const auto& tz = tzones.find(std::get<TZ>(tpl));
  
  return tz::dtime_from_numbers(std::get<Y>(tpl),
                                std::get<M>(tpl),
                                std::get<D>(tpl),
                                std::get<H>(tpl),
                                std::get<MN>(tpl),
                                std::get<SEC>(tpl),
                                std::get<NSEC>(tpl),
                                tz);
}

Global::dtime tz::dtime_from_string(const std::string& s, const std::string &f) {
  // check we consumed all chars LLL
  auto sp = s.c_str();
  auto fp = f.c_str();
  return dtime_from_timetuple(readDtime(sp, sp + s.size(), fp, fp + f.size()));
}


std::string tz::to_string(const tz::interval& i,
                          const std::string& format,
                          const std::string& timezone,
                          bool abbrev,
                          bool fractional)
{
  return
    (i.sopen ? "-" : "+") +
    tz::to_string(i.s, format, timezone, abbrev, fractional) +
    " -> " +
    tz::to_string(i.e, format, timezone, abbrev, fractional) +
    (i.eopen ? "-" : "+");
}


tz::interval tz::interval_from_string(const std::string& s, const std::string &f) {
  const char* sp = s.c_str();
  const char* se = sp + s.size();
  const char* fp = f.c_str();
  const char* fe = fp + f.size();

  bool sopen;
  bool eopen;

  if (*sp == '|') {
    ++sp;
  }
  if (*sp == '-') {
    sopen = true;
  }
  else if (*sp == '+') {
    sopen = false;
  }
  else {
    throw std::range_error("interval open/close must be specified with -/+");
  }
  ++sp;

  skipWhitespace(sp, se);
  const auto is = dtime_from_timetuple(readDtime(sp, se, fp, fe));
  skipWhitespace(sp, se);

  if (*sp++ != '-' || *sp++ != '>') {
    throw std::range_error("interval datetime separator must be '->'");
  }    
  
  skipWhitespace(sp, se);
  fp = f.c_str();
  const auto ie = dtime_from_timetuple(readDtime(sp, se, fp, fe));
  skipWhitespace(sp, se);
                 
  if (*sp == '-') {
    eopen = true;
  }
  else if (*sp == '+') {
    eopen = false;
  }
  else {
    throw std::range_error("interval open/close must be specified with -/+");
  }
  ++sp;

  // check we consumed all chars LLL

  return tz::interval(is, ie, sopen, eopen);
}


tz::Period tz::unqualified_period_from_string(const std::string& s) {
  if (s=="year" || s=="years") 
    return tz::Period::YEAR;  
  else if (s=="month" || s=="months")
    return tz::Period::MONTH;  
  else if (s=="week" || s=="weeks")
    return tz::Period::WEEK;  
  else if (s=="day" || s=="days")
    return tz::Period::DAY;  
  else if (s=="hour" || s=="hours")
    return tz::Period::HOUR;  
  else if (s=="minute" || s=="minutes")
    return tz::Period::MINUTE;  
  else if (s=="second" || s=="seconds")
    return tz::Period::SECOND;  
  else if (s=="millisecond" || s=="milliseconds")
    return tz::Period::MILLI;  
  else if (s=="microsecond" || s=="microseconds")
    return tz::Period::MICRO;  
  else // throw
    throw std::out_of_range("unknown period string");
}


std::string tz::to_string(Global::duration d) {
  std::stringstream ss;

  // handle hh:mm:ss
  if (d < 0s) {
    ss << '-';
    d *= -1;
  }
  auto h = d / 1h;
  d -= h * 3600s;
  auto min = d / 1min;
  d -= min * 60s;
  auto s = d / 1s;
  d -= s * 1s;
  ss << std::setfill ('0') 
     << std::setw(2) << h  << ':' 
     << std::setw(2) << min  << ':' 
     << std::setw(2) << s;
  
  // now handle nanoseconds 000_000_000
  auto ms = d / 1ms;
  d -= ms * 1ms;
  auto us = d / 1us;
  d -= us * 1us;
  auto ns = d / 1ns;
  d -= ns * 1ns;

  if (ms || us || ns) {
    ss << '.';
    ss << std::setfill ('0') << std::setw(3) << ms;
    if (us || ns) {
      ss << '_' << std::setfill('0') << std::setw(3) << us;
      if (ns) {
        ss << '_' << std::setfill('0') << std::setw(3) << ns;
      }
    }
  }

  return ss.str();
}


Global::dtime ztsdb::floor(Global::dtime t, tz::Period p) {
  using namespace std::chrono;
  switch (p) {
  case tz::Period::HOUR:
    return time_point_cast<Global::duration>(time_point_cast<hours>(t));
  case tz::Period::MINUTE:
    return time_point_cast<Global::duration>(time_point_cast<minutes>(t));
  case tz::Period::SECOND:
    return time_point_cast<Global::duration>(time_point_cast<seconds>(t));
  case tz::Period::MILLI:
    return time_point_cast<Global::duration>(time_point_cast<milliseconds>(t));
  case tz::Period::MICRO:
    return time_point_cast<Global::duration>(time_point_cast<microseconds>(t));
  case tz::Period::NANO:
    return time_point_cast<Global::duration>(time_point_cast<nanoseconds>(t));
  default:
    throw std::out_of_range("unknown duration type");
  }
}


Global::dtime ztsdb::floor_tz(Global::dtime t, tz::Period p, const tz::Zone& z) {
  int pos = -1;         // -1 means uninitialized
  int direction = -1;   // floor means we're going backwards in time
  auto t_days = date::floor<date::days>(t + z.getoffset(t, pos, direction));
  switch (p) {
  case tz::Period::DAY:
    return t_days - z.getoffset(t_days, pos, direction);
  case tz::Period::MONTH: {
    auto ymd = date::year_month_day(t_days);
    t_days = date::sys_days(ymd.year()/ymd.month()/date::day(1));
    return t_days - z.getoffset(t_days, pos, direction);
  }
  case tz::Period::YEAR: {
    auto ymd = date::year_month_day(t_days);
    t_days = date::sys_days(ymd.year()/date::month(1)/date::day(1));
    return t_days - z.getoffset(t_days, pos, direction);
  }
  default:
    throw std::out_of_range("unknown duration type");
  }
}


tz::interval ztsdb::floor(tz::interval i, tz::Period p) {
  return tz::interval(floor(i.s, p), floor(i.e, p), i.sopen, i.eopen);
} 


tz::interval ztsdb::floor_tz(tz::interval i, tz::Period p, const tz::Zone& z) {
  return tz::interval(floor_tz(i.s, p, z), floor_tz(i.e, p, z), i.sopen, i.eopen);
} 


Global::dtime ztsdb::ceiling(Global::dtime t, tz::Period p) {
  return ztsdb::floor(t + average_duration(p), p); 
}

tz::interval  ztsdb::ceiling(tz::interval i, tz::Period p) {
  return ztsdb::floor(i + average_duration(p), p);
}

Global::dtime ztsdb::ceiling_tz(Global::dtime t, tz::Period p, const tz::Zone& z) {
  int pos = -1;         // -1 means uninitialized
  int direction = 1;    // celing means we're going forward in time
  auto t_days = date::floor<date::days>(t + z.getoffset(t, pos, direction));
  switch (p) {
  case tz::Period::DAY: {
    auto ymd = date::year_month_day(t_days);
    t_days = date::sys_days(ymd.year()/ymd.month()/++ymd.day());
    return t_days - z.getoffset(t_days, pos, direction);
  }
  case tz::Period::MONTH: {
    auto ymd = date::year_month_day(t_days);
    t_days = date::sys_days(ymd.year()/++ymd.month()/date::day(1));
    return t_days - z.getoffset(t_days, pos, direction);
  }
  case tz::Period::YEAR: {
    auto ymd = date::year_month_day(t_days);
    t_days = date::sys_days(++ymd.year()/date::month(1)/date::day(1));
    return t_days - z.getoffset(t_days, pos, direction);
  }
  default:
    throw std::out_of_range("unknown duration type");
  }
}


tz::interval ztsdb::ceiling_tz(tz::interval i, tz::Period p, const tz::Zone& z) {
  return tz::interval(ceiling_tz(i.s, p, z), ceiling_tz(i.e, p, z), i.sopen, i.eopen);
}


size_t ztsdb::dayweek(Global::dtime t, const tz::Zone& z) {
  // we could have a version that could take advantage of pos and direction... LLL

  int pos = -1;         // -1 means uninitialized
  int direction = 1;
  date::sys_days t_days = 
    date::floor<date::days>(t + z.getoffset(t, pos, direction));

  return unsigned(date::weekday(t_days));
}

size_t ztsdb::daymonth(Global::dtime t, const tz::Zone& z) {
  // we could have a version that could take advantage of pos and direction... LLL
  int pos = -1;         // -1 means uninitialized
  int direction = 1;
  date::sys_days t_days = 
    date::floor<date::days>(t + z.getoffset(t, pos, direction));
  date::weekday wd(t_days);

  return unsigned(date::year_month_day(t_days).day());
}

size_t ztsdb::month(Global::dtime t, const tz::Zone& z) {
  // we could have a version that could take advantage of pos and direction... LLL
  int pos = -1;         // -1 means uninitialized
  int direction = 1;
  date::sys_days t_days = 
    date::floor<date::days>(t + z.getoffset(t, pos, direction));
  date::weekday wd(t_days);

  return unsigned(date::year_month_day(t_days).month());
}

ssize_t ztsdb::year(Global::dtime t, const tz::Zone& z) {
  // we could have a version that could take advantage of pos and direction... LLL
  int pos = -1;         // -1 means uninitialized
  int direction = 1;
  date::sys_days t_days = 
    date::floor<date::days>(t + z.getoffset(t, pos, direction));
  date::weekday wd(t_days);

  return int(date::year_month_day(t_days).year());
}

