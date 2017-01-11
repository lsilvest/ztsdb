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


#ifndef MISC_HPP
#define MISC_HPP


#include <string>
#include <cstdint>
#include <cmath>
#include <endian.h>
#include <string>
#include <stdexcept>
#include <iostream>
#include <functional>
#include <sstream>
#include <netinet/in.h>
#include "globals.hpp"


// LLL put this in an namespace!


// we favour intel CPUs here; what is sent on the wire is not network
// order but little endian; big endian processors will change order
// while little endian processor will have no op.

#if __BYTE_ORDER == __BIG_ENDIAN

inline uint64_t hton64(uint64_t x) { 
  return htobe64(x); 
}

inline uint64_t ntoh64(uint64_t x) { 
  return htobe64(x); 
}

template<typename T>
inline T hton(T x) {
  switch (sizeof(T)) {
  case 4: {
    union { T t; uint32_t i; };
    i = htobe32(reinterpret_cast<uint32_t&>(x));
    return t;
  }
  case 8: {
    union { T t; uint64_t i; };
    i = htobe64(reinterpret_cast<uint64_t&>(x));
    return t;
  }
  default: 
    throw std::domain_error("can't handle hton for element of size: " + std::to_string(sizeof(T)));
  }
}

template<>
inline Global::dtime hton(Global::dtime x) {
  auto i = htobe64(x.time_since_epoch().count());
  auto d = Global::dtime::duration{i};
  return Global::dtime(d);
}

template<typename T>
inline T ntoh(T x) {
  return hton(x);
}


#elif __BYTE_ORDER == __LITTLE_ENDIAN

inline uint64_t hton64(size_t x) { 
  return x; 
}

inline uint64_t ntoh64(size_t x) { 
  return x; 
}

template<typename T>
inline T hton(T x) {
  return x;
}

template<typename T>
inline T ntoh(T x) {
  return x;
}

#else
#error __BYTE_ORDER is neither __LITTLE_ENDIAN nor __BIG_ENDIAN
#endif


std::string printBuf(const char* buf, unsigned len);

inline unsigned getAlignedLength(const std::string& s, unsigned nbyteAlignment) {
  auto rem = s.length() % nbyteAlignment;
  return rem ? s.length() + nbyteAlignment - rem : s.length();
}

inline unsigned getAlignedLength(size_t n, unsigned nbyteAlignment) {
  auto rem = n % nbyteAlignment;
  return rem ? n + nbyteAlignment - rem : n;
}

bool operator<(const sockaddr_in& sa1, const sockaddr_in& sa2);

inline size_t ltostr_r(long val, char* buf, size_t sz) {
  if (val == 0) {
    buf[0] = '0';
    return 1;
  } 
  else {
    size_t e = std::abs(val) <=  1 ? 0 : log10(std::abs(val));
    if (val < 0) { 
      ++e;
      buf[0] = '-';
      val = -val;
    }
    auto idx = e;
    if (idx >= sz) {
      throw std::range_error("ltostr_r: buffer too small");
    }
    while (val > 0) {
      buf[idx] = (val % 10) + '0';
      val /= 10;
      --idx;
    }
    return e + 1;
  }
}

bool readNumber(const char*& s, const char* e, int& n, bool dosign=false);

namespace ztsdb {

  /// This set of functors, to the contrary of the stdlib, allow
  /// operations where the operands and the results are of different
  /// types. This is handy for example is adding duration to time
  /// points, etc.
  template<typename T, typename U, typename R>
  struct plus {
    inline R operator()(const T& t, const U& u) const {
      return t + u;
    }
  };  

  template<typename T, typename U, typename R>
  struct minus {
    inline R operator()(const T& t, const U& u) const {
      return t - u;
    }
  };  

  template<typename T, typename U, typename R>
  struct multiplies {
    inline R operator()(const T& t, const U& u) const {
      return t * u;
    }
  };  

  template<typename T, typename U, typename R>
  struct divides {
    inline R operator()(const T& t, const U& u) const {
      return t / u;
    }
  }; 

  template<typename T, typename U, typename R>
  struct modulus {
    inline R operator()(const T& t, const U& u) const {
      return fmod(t, u);
    }
  }; 

  template<typename T>
  struct max {
    inline T operator()(const T& t, const T& u) const {
      if (std::isnan(t) || std::isnan(u)) {
        return Global::ZNAN;
      }
      return std::max(t, u);
    }
  }; 

  template<typename T>
  struct min {
    inline T operator()(const T& t, const T& u) const {
      if (std::isnan(t) || std::isnan(u)) {
        return Global::ZNAN;
      }
      return std::min(t, u);
    }
  }; 
}


template<typename T>
static inline std::function<T()> GenNbFun() {
  T count = 0;
  return [=]() mutable { return ++count; };
}

inline std::function<std::string(std::string)> GensymFun() {
  auto count = 0;
  return 
    [=](std::string s) mutable {
    std::stringstream ss; 
    ss << s << ++count;
    return ss.str();
  };
}


template <typename Int>
constexpr Int next_power2(Int i) {
  --i;
  Int n = 1;
  while (i > 0) { n <<= 1; i >>= 1; }
  return n;
}


#endif

