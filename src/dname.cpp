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


#include <utility>
#include "dname.hpp"


static void putNamesInMap(const arr::Vector<arr::zstring>& names, 
                          std::map<std::string, std::set<size_t>>& namesMap) 
{
  for (size_t j=0; j<names.size(); ++j) {
    if (names[j] != "") {
      auto e = namesMap.find(names[j]);
      if (e == namesMap.end()) {
        namesMap.emplace(names[j], std::set<size_t>{j});
      } else {
        e->second.insert(j);
      }
    }
  }  
}


arr::Dname::Dname(size_t n, arr::Vector<arr::zstring>&& names_p) : 
  sz(n), names(std::move(names_p)) 
{
  if (names.size() && names.size() != sz) {
    throw std::range_error("length of 'dimnames' not equal to array extent");
  }
  putNamesInMap(names, namesMap);
}


void arr::Dname::resize(size_t n, size_t from) {
  sz = n;
  if (names.size()) {
    names.resize(sz, from, "");
    // redo the map from scratch:
    namesMap.clear();
    putNamesInMap(names, namesMap);
    if (namesMap.size() == 0) {
      // if not set names to 0 size, that's out convention when we
      // have no names and this makes comparisons easier:
      names.resize(0);
    }
  } 
}


void arr::Dname::assign(size_t i, const std::string& s) {
  if (i >= sz) {
    throw std::range_error("index out of bound");
  }
  if (s == "" && !names.size()) {
    return;
  }
  if (!hasNames()) {
    names.init(sz, "");
    setv(names, i, arr::zstring(s));
  }
  else {
    setv(names, i, arr::zstring(s));
    namesMap.clear();
    putNamesInMap(names, namesMap);
    // check we still have names:
    if (namesMap.size() == 0) {
      // if not set names to 0 size, that's out convention when we
      // have no names and this makes comparisons easier:
      names.resize(0);
    }
  }
}


void arr::Dname::addafter(const std::string& s) {
  if (s == "" && !names.size()) {
    ++sz;
  }
  else {
    if (!names.size()) {
      names.init(sz, "");
    } 
    names.push_back(s);
    
    auto e = namesMap.find(s);
    if (e == namesMap.end()) {
      namesMap.emplace(s, std::set<size_t>{sz});
    } else {
      e->second.insert(sz);
    }
    ++sz;
  }
}


void arr::Dname::addafter(const Dname& dn) {
  if (!names.size() && dn.names.size()) {
    names.init(sz, "");
    names.insert(names.end(), dn.names.begin(), dn.names.end());
  } else if (names.size() && !dn.names.size()) {
    // we already have names, so make sure the new extent is inialized
    // even if dn is empty:
    std::vector<zstring> nostrings(dn.sz, "");
    names.insert(names.end(), nostrings.begin(), nostrings.end());
  } else if (names.size() && dn.names.size()) {
    names.insert(names.end(), dn.names.begin(), dn.names.end());
  } // else neither have names, so leave names as is
  
  for (size_t j=0; j<dn.names.size(); ++j) {
    auto e = namesMap.find(dn.names[j]);
    if (e == namesMap.end()) {
      namesMap.emplace(dn.names[j], std::set<size_t>{sz+j});
    } else {
      e->second.insert(sz+j);
    }
  }
  sz += dn.sz;
}


void arr::Dname::remove(size_t n, const std::string& nm) {
  names.erase(names.begin() + n);
  // redo the map from scratch:
  namesMap.clear();
  putNamesInMap(names, namesMap);
  if (namesMap.size() == 0) {
    // if not set names to 0 size, that's out convention when we
    // have no names and this makes comparisons easier:
    names.resize(0);
  }
  --sz;
}


void arr::Dname::remove(size_t n) {
  std::string nm = (*this)[n];       // will throw if out of bound
  remove(n, nm);
}


void arr::Dname::remove(const std::string& nm) {
  size_t n = (*this)[nm]; // will throw if out of bound
  remove(n, nm);
}


void arr::Dname::addprefix(const std::string& prefix) {
  if (prefix.length()) {
    if (sz != names.size()) {
      names.init(sz, "");
    }
    for (std::vector<Dname>::size_type i=0; i<names.size(); ++i) {
      if (names[i] != "") {
        setv(names, i, prefix + "." + names[i]);
      } else {
        if (names.size() == 1) {
          setv(names, i, zstring(prefix));
        } 
        else {
          setv(names, i, zstring(prefix + std::to_string(i+1)));
        }
      }
    }
    // have to redo the map from scratch:
    namesMap.clear();
    putNamesInMap(names, namesMap); 
  }
}


bool arr::Dname::operator==(const Dname& d) const {
  // std::cout << "sz: " << sz << " d.sz: " << d.sz << std::endl;
  return sz == d.sz && names == d.names;
}


size_t arr::Dname::operator[](const std::string& s) const {
  auto e = namesMap.find(s);
  if (e == namesMap.end()) {
    throw std::range_error("subscript out of bounds");    
  } else {
    return *(e->second.begin());
  }  
}


std::string arr::Dname::operator[](size_t i) const {
  if (i >= sz) {
    throw std::range_error("subscript out of bounds");    
  }
  else if (i >= names.size()) {
    return "";
  }
  else {
    return names[i];
  }
}


