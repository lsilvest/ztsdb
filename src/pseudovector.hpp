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


#ifndef PSEUDO_VECTOR_HPP
#define PSEUDO_VECTOR_HPP


#include "vector.hpp"


namespace arr { 

  template <typename T, typename U=T>
  struct PseudoVector {
    PseudoVector(const Vector<U>& v_p, size_t sz_p=0) : 
      v(v_p), scalar(v.size() == 1), first_elt(v[0]), sz(sz_p) { }
    inline const U operator[](size_t i) const { return scalar ? first_elt : v[i]; }
    
    inline const T plus(T t, U u) const { return ztsdb::plus<T,U,T>()(t, u); }
    inline size_t size() const { return scalar ? sz : v.size(); }

  private:
    const Vector<U>& v;
    const bool scalar;
    const U first_elt;
    const size_t sz;
  };

} // end namespace arr


#endif
