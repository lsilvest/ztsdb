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


#ifndef ZSTRING_HPP
#define ZSTRING_HPP


#include <string>
#include <cstring>
#include <iostream>
#include <stdexcept>


namespace arr {

  const size_t DEFAULT_STRING_SIZE = 128;

  template<int S=DEFAULT_STRING_SIZE> // why int LLL
  struct ZString {
    static const size_t STRING_SIZE = S;

    template<int U>
    friend std::ostream& operator<<(std::ostream& os, const ZString<U>& s);
    template<int U>
    friend ZString<U> operator+(const ZString<U>& s1, const ZString<U>& s2);
    template<int U>
    friend ZString<U> operator+(const ZString<U>& s1, const char* s2);
    template<int U>
    friend ZString<U> operator+(const char* s1, const ZString<U>& s2);
    template<int U>
    friend ZString<U> operator+(const ZString<U>& s1, const std::string& s2);
    template<int U>
    friend ZString<U> operator+(const std::string& s1, const ZString<U>& s2);
    template<int U>
    friend bool operator==(const ZString<U>& s1, const ZString<U>& s2);
    template<int U>
    friend bool operator!=(const ZString<U>& s1, const ZString<U>& s2);
    template<int U>
    friend bool operator<=(const ZString<U>& s1, const ZString<U>& s2);
    template<int U>
    friend bool operator<(const ZString<U>& s1, const ZString<U>& s2);
    template<int U>
    friend bool operator>=(const ZString<U>& s1, const ZString<U>& s2);
    template<int U>
    friend bool operator>(const ZString<U>& s1, const ZString<U>& s2);
    template<int U>
    friend bool operator==(const ZString<U>& s1, const char* s2);
    template<int U>
    friend bool operator!=(const ZString<U>& s1, const char* s2);
    template<int U>
    friend bool operator<=(const ZString<U>& s1, const char* s2);
    template<int U>
    friend bool operator<(const ZString<U>& s1, const char* s2);
    template<int U>
    friend bool operator>=(const ZString<U>& s1, const char* s2);
    template<int U>
    friend bool operator>(const ZString<U>& s1, const char* s2);

    ZString() { v[0] = 0; }
    ZString(const char* s) { strncpy(v, s, S-1); }
    ZString(const char* s, const char* e) { 
      memcpy(v, s, std::min(e-s, static_cast<ssize_t>(S-1)));
      v[std::min(e-s, static_cast<ssize_t>(S-1))] = 0; 
    }
    ZString(const std::string& s) { strncpy(v, s.c_str(), S-1); }
    ZString(const ZString& s) { 
      strncpy(v, s.v, S-1); 
    }
    ZString(char c, size_t n) { 
      auto len = std::min(n, static_cast<size_t>(S-1));
      memset(v, c, len); 
      v[len] = '\0';
    }

    ZString& operator=(const ZString& s) { strncpy(v, s.v, S-1); return *this; }
    operator std::string() const { return v; }  

    inline char operator[](unsigned pos) const { 
      if (pos < size()) return v[pos];
      else throw std::range_error("zstring out of boundary subset");
    }
    size_t find (char c, size_t pos = 0) const {
      while (pos < size() && pos < S && v[pos] != c) {
        ++pos;
      }
      return pos;
    }
    const char* c_str() const { return v; }

    size_t length() const { return strlen(v); }
    size_t size()   const { return strlen(v); }

  private:
    char v[S];
  };

  template<int U>
  std::ostream& operator<<(std::ostream& os, const ZString<U>& s) {
    os << s.v;
    return os;
  }
  
  template<int U>
  ZString<U> operator+(const ZString<U>& s1, const ZString<U>& s2) {
    ZString<U> s(s1);
    strncpy(&s.v[s.length()], s2.v, U - s.length() - 1);
    return s;
  }
  template<int U>
  ZString<U> operator+(const ZString<U>& s1, const char* s2) {
    return s1 + ZString<U>(s2);
  }
  template<int U>
  ZString<U> operator+(const char* s1, const ZString<U>& s2) {
    return ZString<U>(s1) + s2;
  }
  template<int U>
  ZString<U> operator+(const ZString<U>& s1, const std::string& s2) {
    return s1 + ZString<U>(s2);
  }
  template<int U>
  ZString<U> operator+(const std::string& s1, const ZString<U>& s2) {
    return ZString<U>(s1) + s2;
  }

  template<int U>
  bool operator==(const ZString<U>& s1, const ZString<U>& s2) {
    return strncmp(s1.v, s2.v, U) == 0;
  }
  template<int U>
  bool operator!=(const ZString<U>& s1, const ZString<U>& s2) {
    return strncmp(s1.v, s2.v, U) != 0;
  }
  template<int U>
  bool operator<=(const ZString<U>& s1, const ZString<U>& s2) {
    return strncmp(s1.v, s2.v, U) <= 0;
  }
  template<int U>
  bool operator<(const ZString<U>& s1, const ZString<U>& s2) {
    return strncmp(s1.v, s2.v, U) < 0;
  }
  template<int U>
  bool operator>=(const ZString<U>& s1, const ZString<U>& s2) {
    return strncmp(s1.v, s2.v, U) >= 0;
  }
  template<int U>
  bool operator>(const ZString<U>& s1, const ZString<U>& s2) {
    return strncmp(s1.v, s2.v, U) > 0;
  }

  template<int U>
  bool operator==(const ZString<U>& s1, const char* s2) {
    return strncmp(s1.v, s2, U) == 0;
  }
  template<int U>
  bool operator!=(const ZString<U>& s1, const char* s2) {
    return strncmp(s1.v, s2, U) != 0;
  }
  template<int U>
  bool operator<=(const ZString<U>& s1, const char* s2) {
    return strncmp(s1.v, s2, U) <= 0;
  }
  template<int U>
  bool operator<(const ZString<U>& s1, const char* s2) {
    return strncmp(s1.v, s2, U) < 0;
  }
  template<int U>
  bool operator>=(const ZString<U>& s1, const char* s2) {
    return strncmp(s1.v, s2, U) >= 0;
  }
  template<int U>
  bool operator>(const ZString<U>& s1, const char* s2) {
    return strncmp(s1.v, s2, U) > 0;
  }

  using zstring = ZString<>;


}


#endif
