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


#include <sstream>
#include <cmath>
#include <regex>
#include "misc.hpp"


std::string printBuf(const char* buf, unsigned len) {
  std::stringstream ss;
  for (unsigned i=0; i<len; ++i) {
    ss.fill('0');
    ss.width(2);
    ss << std::hex << unsigned((unsigned char)(buf[i])) << ' ';
    if (i && (i+1) % 16 == 0) {
      ss << std::endl;
    }
  }
  return ss.str();
}


bool operator<(const sockaddr_in& sa1, const sockaddr_in& sa2) {
  return sa1.sin_addr.s_addr < sa2.sin_addr.s_addr ? true :
    (sa1.sin_addr.s_addr == sa2.sin_addr.s_addr ? 
     sa1.sin_port < sa2.sin_port : false);
}


bool readNumber(const char*& s, const char* e, int& n, bool dosign) {
  n = 1;
  auto sorig = s;
  int sign = 1;
  if (dosign && *s == '-') {
    sign = -1;
    ++s;
  }
  if (s == e || !isdigit(*s)) {
    s = sorig;
    return false;
  }
  else {
    n *= *s - '0';
    s++;
  }
   
  while (s < e && *s >= '0' && *s <= '9') {
    n = 10*n + (*s - '0');
    ++s;
  } 

  n *= sign;
  return true;
}
