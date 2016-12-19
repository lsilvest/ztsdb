// (C) 2015 Leonardo Silvestri
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


#ifndef DNAME_H
#define DNAME_H

#include <map>
#include <sys/types.h>
#include "vector.hpp"
#include "string.hpp"


namespace arr {

  /// Handling of dimension names. The specificities are that names
  /// are not unique and can be missing.
  struct Dname {
    Dname(size_t n=0, Vector<zstring>&& names_p=Vector<zstring>());

    void resize(size_t n, size_t from=0);
    void assign(size_t i, const std::string& s);
    void addafter(const std::string& s);
    void addafter(const Dname& dn);
    void remove(size_t n, const std::string& nm);
    void remove(size_t n);
    void remove(const std::string& nm);
    void addprefix(const std::string& prefix);
    bool hasNames() const { return sz && sz == names.size(); }

    bool   operator==(const Dname& d) const;
    bool   operator!=(const Dname& d) { return !(*this == d); }
    size_t operator[](const std::string& s) const; // if multiple s, returns the first
    std::string   operator[](size_t i) const;

    inline size_t size() const { return names.size(); }

    Vector<zstring>::iterator begin() { return names.begin(); }
    Vector<zstring>::iterator end()   { return names.end(); }

    size_t sz;                // the size of the index
    Vector<zstring> names;    
    std::map<std::string, std::set<size_t>> namesMap;   
  };

}

#endif
