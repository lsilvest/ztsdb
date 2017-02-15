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


#ifndef LOGGING_HPP
#define LOGGING_HPP


#include <cstdio>
#include <cstdarg>
#include <mutex>
#include "timezone/ztime.hpp"


extern tz::Zones tzones;


using namespace std::string_literals;


namespace zlog {

  enum Severity {
    SV_TRACE,
    SV_DEBUG,
    SV_INFO,
    SV_WARN,
    SV_ERROR,
    SV_FATAL
  };


  inline const char* to_string(Severity sv) {
    static const char* svstr[] = {
      "TRACE",
      "DEBUG",
      "INFO",
      "WARN",
      "ERROR",
      "FATAL"
    };
    return static_cast<std::size_t>(sv) < sizeof(svstr)/sizeof(svstr[0]) ? 
                                          svstr[static_cast<int>(sv)] : "unknown";
  }

  inline Severity from_string(const std::string& s) {
    if (s == "TRACE"s) return SV_TRACE;
    if (s == "DEBUG"s) return SV_DEBUG;
    if (s == "INFO"s)  return SV_INFO;
    if (s == "WARN"s)  return SV_WARN;
    if (s == "ERROR"s) return SV_ERROR;
    if (s == "FATAL"s) return SV_FATAL;
    throw std::range_error("unknown severity string");
  }

  // Besides severity filtering, we could also implement tag
  // filtering, so we then have the possibility to enable logging only
  // for specific (tagged) modules. For the moment we rely on local
  // '#define" for debug information (which gets compiled out).

  /// Very simple logger. It's quite enough though as we don't use it
  /// on time-critical paths (for release versions at least).
  struct Logger {
    Logger() : f(nullptr), level(SV_TRACE), tz("GMT") { }
    Logger(const std::string& s, const std::string& tz_p) : level(SV_TRACE) {
      init(s, tz_p);
    }

    inline void init(const std::string& s, const std::string& tz_p) {
      f = fopen(s.c_str(), "a");
      if (f == nullptr) {
        throw std::system_error(std::error_code(errno, std::system_category()), 
                                "cannot open log file");        
      }
      tz = tz_p;
    }

    inline void uninit() {
      if (f) {
        fflush(f);
        fclose(f);
        f = nullptr;   
      }
      tz = "GMT";
    }
    
    /// Close the current log file if any and open the specified file
    /// for logging.
    inline int changeFile(const std::string& s) {
      if (f) {
        fclose(f);
        f = nullptr;
      }
      if (!s.empty()) {
        f = fopen(s.c_str(), "a");
        if (f == nullptr) {
          return errno;
        }
      }
      return 0;
    }

    inline int log(Severity sv, const char* fmt, ...) {
      if (sv >= level) {
        va_list ap;
        va_start(ap, fmt);
        std::string tm = tz::to_string(std::chrono::system_clock::now(),
                                       "", tzones.find(tz), tz, true); 
        static char str[BUFLEN];
        mx.lock();              // to protect 'str'
        vsnprintf(str, BUFLEN, fmt, ap);
        int res = f ? 
          fprintf(f, "%s [%s]: %s\n", tm.c_str(), to_string(sv), str) : 
          printf("%s [%s]: %s\n", tm.c_str(), to_string(sv), str);
        if (f) fflush(f);       // because apparently, in C++ with
                                // gcc/CLANG, flush doesn't
                                // automatically occur after newline
        mx.unlock();
        va_end(ap);
        return res;
      }
      else {
        return 0;
      }
    }

    inline void setLevel(Severity sv) { level = sv; }

    ~Logger() {
      uninit();
    }

    inline Severity from_string(const std::string& s) {
      if (s == "TRACE"s) return SV_TRACE;
      if (s == "DEBUG"s) return SV_DEBUG;
      if (s == "INFO"s)  return SV_INFO;
      if (s == "WARN"s)  return SV_WARN;
      if (s == "ERROR"s) return SV_ERROR;
      if (s == "FATAL"s) return SV_FATAL;
      throw std::range_error("unknown severity string");
    }

  private:
    FILE* f;
    Severity level;
    std::mutex mx;
    std::string tz;
    static const int BUFLEN = 512;
  };


}

#endif
