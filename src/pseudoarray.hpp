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


#ifndef PSEUDO_ARRAY_HPP
#define PSEUDO_ARRAY_HPP


#include "array.hpp"


namespace arr {

  template <typename T, typename O=typename Array<T>::comparator>
  struct PseudoArray {
    PseudoArray(const Array<T,O>& v_p, size_t cols_p=0, size_t sz_p=0) : 
      v(v_p), scalar(v.size() == 1), first_elt(v[0]), 
      cols(cols_p == 0 ? v.size() : cols_p), sz(sz_p == 0 ? v.size() : sz_p), colstruct(v[0]) { }
    
    inline const T operator[](size_t i) const { return scalar ? first_elt : v[i]; }

    inline size_t size() const { return sz; }
    template <typename C>
    inline const C& getcol(size_t i) { return scalar ? colstruct : v.getcol(i); }

    template <typename V>
    struct ColStruct {
      ColStruct(const V& v_p) : v(v_p) { }
      const V& operator[](size_t i) const { return v; }
    private:
      const V& v;
    };

  private:
    const Array<T,O>& v;
    const bool scalar;
    const T first_elt;
    const size_t cols;
    const size_t sz;
    const ColStruct<T> colstruct;
  };

} // end namespace arr


#endif
