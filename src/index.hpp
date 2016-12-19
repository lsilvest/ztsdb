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


#ifndef INDEX_HPP
#define INDEX_HPP

#include <cstddef>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <iostream>
#include "juice/variant.hpp"
#include "globals.hpp"
#include "vector.hpp"
#include "dname.hpp"
#include "timezone/interval.hpp"


using namespace Juice;

namespace arr {


  template<typename I, typename T, typename U>
  void subassign_generic(const I& idx, 
                         Vector<T>& rv, 
                         const U& v,
                         idx_type& vj);
  template<typename I, typename T, typename U>
  inline void subassignScalar_generic(const I& idx, Vector<T>& rv, U u);
  
  /// We define a series of classes that implement the various types
  /// of indices. We do not derive them from a base class and use
  /// virtual functions, instead we use them in a Variant for
  /// performance reasons. Note that the XyzIndex always holds all the
  /// information (via a const ref) to any data it needs in order to
  /// convert to indices of type idx_type. This is done by the inline
  /// functions getfirst and getnext.
  struct NullIndex {
    const idx_type sz;

    inline bool getfirst(idx_type& val, idx_type& i) const {
      i = val = 0;
      return sz > 0;
    }

    inline bool getnext(idx_type& val, idx_type& i) const {
      val = ++i;
      if (i < sz) {
        return true;
      } else {
        i = val = 0;
        return false;
      }
    }
    inline size_t trueSize() const { return size(); }
    inline size_t size() const { return sz; }

    /// Subset 'v' according to this index and put the result into 'rv'.
    template<typename T>
    inline void subset(Vector<T>& rv, const Vector<T>& v) const {
      rv = v;
    }

    /// Subassign from 'v' into 'rv' according to index conceptually
    /// linked to 'rv'. i.e. this is taking 'v' and putting it into
    /// the places in 'rv' indicated by this index.
    template<typename T, typename U>
    void subassign(Vector<T>& rv, 
                   U& u,
                   idx_type& vj) const {
      for (idx_type j=0; j<rv.size(); ++j) {
        setv_checkbefore(rv, j, convert<T,typename U::value_type>(u[vj++]));
      }
    }

    /// Subassign the scalar 'u' to 'rv' according to index conceptually
    /// linked to 'rv'. i.e. this is taking 'u' and putting it into
    /// the places in 'rv' indicated by this index.
    template<typename T, typename U>
    inline void subassignScalar(Vector<T>& rv, U u) const {
      for (idx_type j=0; j<rv.size(); ++j) {
        setv(rv, j, convert<T,U>(u));
      }
    } 

    inline void selectNames(Dname& tonames, const Dname& fromnames) const {
      tonames = fromnames;      
    }
  };

  
  template <typename I>
  struct IntIndex_T {
    IntIndex_T(const Vector<I>& vi_p) : vi(vi_p), vidx(vi.size()), uidx(vi.size()) {
      if (vi.isOrdered()) {
        vidx = vi;
        std::iota(uidx.begin(), uidx.end(), 0);
      }
      else {
        const auto& idx = vi.template sort_idx<size_t>();
        for (idx_type j=0; j<idx.size(); ++j) {
          setv(vidx, idx[j], vi[j]);
          setv(uidx, idx[j], j);
        }
      }
    }
    
    Vector<I> vi;
    Vector<size_t> vidx;
    Vector<size_t> uidx;

    bool getfirst(idx_type& iv, idx_type& ii) const {
      if (vi.size()) {
        ii = 0;
        iv = vi[0];
        return true;
      }
      else {
        return false;
      }
    }

    bool getnext(idx_type& iv, idx_type& ii) const {
      if (vi.size()) {
        if (++ii < vi.size()) {
          iv = vi[ii];
          return true;
        } else {
          getfirst(iv, ii);
          return false;
        }
      }
      else {
        return false;
      }
    }

    inline size_t trueSize() const { 
      return vi.size(); 
    }
    inline size_t size() const { 
      return trueSize(); 
    }

    /// Subset 'v' and store in 'rv'.
    template<typename T>
    inline void subset(Vector<T>& rv, const Vector<T>& v) const {
      for (idx_type j=0; j<vi.size(); ++j) {
        if (vi[j] < 0) throw range_error("mixed positive and negative subscripts");
        if (vi[j] >= v.size()) throw range_error("subscript out of bounds");
        rv.push_back(v[vi[j]]);
      }
    }

    /// Subset and store state in 'ii' (index for index). 'pos' is the
    /// index into 'v' when considered as a vector. This is useful in
    /// order to subset a matrix as if it were a vector. This function
    /// is called repeatedly on the columns of the matrix.
    template<typename T>
    inline void subset(Vector<T>& rv, const Vector<T>& v, idx_type& ii, idx_type pos) const {
      for (; ii<vi.size(); ++ii) {
        if (vi[ii] < 0) throw range_error("mixed positive and negative subscripts");
        // if no index falls inside the extent of v, then return:
        if (vi[ii] < pos || vi[ii] >= v.size() + pos) return;
        rv.push_back(v[vi[ii]-pos]);
      }
    }

    /// We need to reorder vi (and index accordingly into v) because
    /// this is the only way to determine the ordering is correct
    /// after subassign (excluding checking the whole of the vector
    /// which is unacceptedly expensive).
    template<typename T, typename U>
    void subassign(Vector<T>& rv, 
                   const U& u,
                   idx_type& vj) const {
      for (idx_type j=0; j<vi.size(); ++j) {
        j+1 < vi.size() && vidx[j+1] - vidx[j] == 1 ?
              setv_checkbefore(rv, vidx[j], convert<T,typename U::value_type>(u[vj+uidx[j]])) :
              setv(rv, vidx[j], convert<T,typename U::value_type>(u[vj+uidx[j]]));
      }
      vj += vi.size();
    }

    template<typename T, typename U>
    inline void subassignScalar(Vector<T>& rv, U u) const { 
      for (idx_type j=0; j<vi.size(); ++j) {
        j+1 < vi.size() && vidx[j+1] - vidx[j] == 1 ?
          setv_checkbefore(rv, vidx[j], convert<T,U>(u)) :
          setv(rv, vidx[j], convert<T,U>(u));
      }
    }

    inline void selectNames(Dname& tonames, const Dname& fromnames) const {
      if (fromnames.names.size() > 0) {
        for (idx_type j=0; j<vi.size(); ++j) {
          if (fromnames.sz <= vi[j]) {
            throw range_error("subscript out of bounds");
          } else {
            tonames.addafter(fromnames.names[vi[j]]);
          }
        }
      } else {
        tonames = Dname(vi.size());
      }
    }  
  }; // struct IntIndex


  template <typename I>
  struct IntIndexNeg_T {
    IntIndexNeg_T(const Vector<I>& vi_p, idx_type sz_p) : vi(vi_p), sz(sz_p) 
    {
      if (vi.size() && !vi.isOrdered()) {
        vi.sort();
        vi.erase(unique(vi.begin(), vi.end()), vi.end());
      }
      if (vi.size() && (vi.back() >= sz || !sz)) throw range_error("subscript out of bounds");
    }
    
    Vector<I> vi;
    idx_type sz;

    bool getfirst(idx_type& iv, idx_type& ii) const {
      ii = iv = 0;
      while (ii < vi.size() && vi[ii] == iv) { 
        ++ii; 
        if (iv < sz) ++iv; 
      }
      if (iv == sz) {
        ii = iv = 0;
        return false;
      }
      else {
        return true;
      }
    }

    bool getnext(idx_type& iv, idx_type& ii) const {
      ++iv;
      while (ii < vi.size() && vi[ii] == iv) {
        ++ii;
        if (iv < sz) ++iv; 
      }
      if (iv == sz) {
        getfirst(iv, ii);     // reset, this is needed by the getcol functions
        return false;
      }
      else {
        return true;
      }
    }

    inline size_t trueSize() const {
      // the following is always correct as the constructor rejects
      // any vi value that is out of 0..sz-1:
      return sz - vi.size();
    }
    inline size_t size() const { 
      return trueSize(); 
    }

    /// Subset 'v' and store in 'rv'.
    template<typename T>
    inline void subset(Vector<T>& rv, const Vector<T>& v) const {
      idx_type ii = 0;
      for (idx_type j=0; j<sz; ++j) {
        if (ii < vi.size() && vi[ii] == j)
          ++ii;
        else
          rv.push_back(v[j]);
      }
    }

    /// Subset and store state in 'ii' (index for index). 'iv' is the
    /// index into 'v' when considered as a vector. This is useful in
    /// order to subset a matrix as if it were a vector. This function
    /// is called repeatedly on the columns of the matrix.
    template<typename T>
    inline void subset(Vector<T>& rv, const Vector<T>& v, 
                       idx_type& ii, idx_type& iv, idx_type pos) const {
      for (; iv<sz; ++iv) {
        if (iv >= pos + v.size()) return;
        if (ii < vi.size() && vi[ii] == iv)
          ++ii;
        else
          rv.push_back(v[iv - pos]);
      }
    }

    template<typename T, typename U>
    void subassign(Vector<T>& rv, 
                   U& u,
                   idx_type& vj) const {
      subassign_generic(*this, rv, u, vj);
    }

    template<typename T, typename U>
    inline void subassignScalar(Vector<T>& rv, U u) const { 
      subassignScalar_generic(*this, rv, u);
    }

    inline void selectNames(Dname& tonames, const Dname& fromnames) const {
      idx_type ii = 0;
      if (fromnames.names.size() > 0) {
        for (idx_type j=0; j<sz; ++j) {
          if (fromnames.sz <= j) {
            throw range_error("subscript out of bounds");
          }
          if (!(ii < vi.size() && vi[ii] == j)) {
            tonames.addafter(fromnames.names[j]);
          }
          else {
            ++ii;
          }
        }
      } else {
        tonames = Dname(size());
      }
    }  
  };  // end struct IntIndexNeg_T


  struct NameIndex {
    const vector<string> vs;
    const Dname& names;

    bool getfirst(idx_type& val, idx_type& i) const {
      if (vs.size()) {
        i = 0;
        val = names[vs[0]];
        return true;
      } else {
        return false;
      } 
    }

    bool getnext(idx_type& val, idx_type& i) const {
      if (++i < vs.size()) {
        val = names[vs[i]];
        return true;
      } else {
        getfirst(val, i);
        return false;
      }
    }

    inline size_t trueSize() const { return vs.size(); }
    inline size_t size() const { return vs.size(); }

    template<typename T>
    inline void subset(Vector<T>& rv, const Vector<T>& v) const {
      for (idx_type j=0; j<vs.size(); ++j) {
        rv.push_back(v[names[vs[j]]]);
      }
    }

    template<typename T, typename U>
    void subassign(Vector<T>& rv, 
                   const U& u,
                   idx_type& vj) const {
      subassign_generic(*this, rv, u, vj);
    }

    template<typename T, typename U>
    inline void subassignScalar(Vector<T>& rv, U u) const {
      subassignScalar_generic(*this, rv, u);
    }

    inline void selectNames(Dname& tonames, const Dname& fromnames) const {
      for (idx_type j=0; j<vs.size(); ++j) {
        fromnames[vs[j]]; // really just to check if it's there...
        tonames.addafter(vs[j]);
      }
    }
  }; // end struct NameIndex

  struct BoolIndex {
    const Vector<bool>& vb;

    bool getfirst(idx_type& val, idx_type& i) const {
      for (i=0; i<vb.size(); ++i) {
        if (vb[i]) {
          val = i;
          return true;
        }
      }
      return false;
    }

    bool getnext(idx_type& val, idx_type& i) const {
      for (; ++i<vb.size(); ++i) {
        if (vb[i]) {
          val = i;
          return true;
        }
      }
      getfirst(val, i);
      return false;
    }

    inline size_t trueSize() const { return size(); }
    inline size_t size() const { return std::count(vb.cbegin(), vb.cend(), true); }

    template<typename T>
    inline void subset(Vector<T>& rv, const Vector<T>& v) const { 
      for (idx_type j=0; j<vb.size(); ++j) {
        if (vb[j]) {
          rv.push_back(v[j]);
        }
      }
    }

    /// Subset and store state in 'ii' (index for index). 'iv' is the
    /// index into 'v' when considered as a vector. This is useful in
    /// order to subset a matrix as if it were a vector. This function
    /// is called repeatedly on the columns of the matrix.
    template<typename T>
    inline void subset(Vector<T>& rv, const Vector<T>& v, idx_type& ii) const {
      for (idx_type j=0; j<v.size(); ++j) {
        if (ii >= vb.size()) {
          throw range_error("subscript out of bounds");
        }         
        if (vb[ii++]) {
          rv.push_back(v[j]);
        }
      }
    }

    template<typename T, typename U>
    void subassign(Vector<T>& rv, 
                   const U& u,
                   idx_type& vj) const {
      subassign_generic(*this, rv, u, vj);
    }

    template<typename T, typename U>
    inline void subassignScalar(Vector<T>& rv, U u) const { 
      subassignScalar_generic(*this, rv, u);
    }

    inline void selectNames(Dname& tonames, const Dname& fromnames) const {
      if (fromnames.names.size() > 0) {
        for (idx_type j=0; j<vb.size(); ++j) {
          if (vb[j]) {
            if (fromnames.sz <= j) {
              throw range_error("subscript out of bounds");
            } else {
              tonames.addafter(fromnames.names[j]);
            }
          }
        }
      } else {
        idx_type nb = size(); // get the number of elements set to true
        tonames = Dname(nb);
      }
    }
  };  // end struct BoolIndex

  struct DtimeIndex {
    const Vector<Global::dtime>& idx;  // index
    const Vector<Global::dtime>& vd;

    // note: we don't make an assumption on the ordering of idx

    inline bool getfirst(idx_type& val, idx_type& i) const {
      while (i < idx.size()) {
        auto p = bsearch(&idx[i], vd.c_ptr(), vd.size(), sizeof(Global::dtime), comp);
        if (p) {
          val = static_cast<Global::dtime*>(p) - vd.c_ptr();
          return true;
        }
        else {
          ++i;
        }
      }
      return false;
    }

    inline bool getnext(idx_type& val, idx_type& i) const {
      return getfirst(val, ++i);
    }

    inline size_t trueSize() const { // LLL really pathetic!
      idx_type ii = 0, iv = 0;
      size_t n = 0;
      while (iv < vd.size() && ii < idx.size()) {
        if (vd[iv] < idx[ii]) {
          ++iv;
        } else if (vd[iv] > idx[ii]) {
          ++ii;
        } else { 
          ++iv;
          ++ii;
          ++n;
        }
      }
      return n;
    }
    
    inline size_t size() const { 
      // because we don't know in advance the true size of the subset,
      // for the moment we send back the maximum size LLL
      return idx.size(); 
    }

    template<typename T>
    inline void subset(Vector<T>& rv, const Vector<T>& v) const {
      if (vd.size() != v.size()) {
        throw range_error("size mismatch between vector to subset and time index");
      }
      idx_type i = 0;
      idx_type val;
      if (getfirst(val, i)) {
        rv.push_back(v[val]);
        while (getnext(val, i)) {
          rv.push_back(v[val]);
        }
      }
    }

    template<typename T, typename U>
    void subassign(Vector<T>& rv, 
                   const U& u,
                   idx_type& vj) const { 
      subassign_generic(*this, rv, u, vj);
    }

    template<typename T, typename U>
    inline void subassignScalar(Vector<T>& rv, U u) const { 
      subassignScalar_generic(*this, rv, u);
    }

    inline void selectNames(Dname& tonames, const Dname& fromnames) const {
      if (fromnames.names.size() > 0) {
        idx_type ii = 0, iv = 0;
        while (iv < vd.size() && ii < idx.size()) {
          if (vd[iv] < idx[ii]) {
            ++iv;
          } else if (vd[iv] > idx[ii]) {
            ++ii;
          } else { 
            tonames.addafter(fromnames.names[iv++]);
            ++ii;
          }
        }     
      } else {
        idx_type nb = size(); // get the number of elements set to true
        tonames = Dname(nb);
      }
    }
    
  private:
    /// Comparison function that satisfies the requirements of the C library's 'bsearch'.
    static inline int comp(const void* a, const void* b)
    {
      if (*static_cast<const Global::dtime*>(a) < *static_cast<const Global::dtime*>(b)) return -1;
      if (*static_cast<const Global::dtime*>(a) > *static_cast<const Global::dtime*>(b)) return  1;
      return 0;
    }
  }; // end struct DtimeIndex
  

  /// Provides the indexing into a 'Global::dtime' vector with an
  /// interval index. That's the only thing we allow an interval to
  /// index into. In particular, the indexing into an interval vector
  /// is not provided because the elements of this interval vector
  /// would not only be selected, they would actually change, thus
  /// changing the semantics of indexing.
  struct IntervalIndex {
    const Vector<tz::interval>& idx;  // index
    const Vector<Global::dtime>& vi;

    // note: we don't make an assumption on the ordering of idx

    inline bool getfirst(idx_type& val, idx_type& i) const {
      while (i < idx.size()) {
        const auto iter = idx[i].sopen ? 
          std::upper_bound(vi.begin(), vi.end(), idx[i].s) :
          std::lower_bound(vi.begin(), vi.end(), idx[i].s);
        val = iter - vi.begin();
        if (iter == vi.end() || (!idx[i].eopen ? vi[val] > idx[i].e : vi[val] >= idx[i].e)) {
          ++i;
        }
        else {
          return true;
        }
      }
      return false;
    }
   
    inline bool getnext(idx_type& val, idx_type& i) const {
      ++val;
      while (val < vi.size() && i < idx.size()) {
        if (!idx[i].sopen ? vi[val] < idx[i].s : vi[val] <= idx[i].s) {
          ++val;
        } else if (!idx[i].eopen ? vi[val] > idx[i].e : vi[val] >= idx[i].e) {
          if (!getfirst(val, ++i)) {
            return false;
          }
          else {
            return true;
          }
        } else {
          return true;
        }  
      }
      return false;
    }
   
    inline size_t trueSize() const { 
      size_t n = 0;
      idx_type iv=0, ii=0;
      while (iv < vi.size() && ii < idx.size()) {
        if (!idx[ii].sopen ? vi[iv] < idx[ii].s : vi[iv] <= idx[ii].s) {
          ++iv;
        } else if (!idx[ii].eopen ? vi[iv] > idx[ii].e : vi[iv] >= idx[ii].e) {
          ++ii;
        } else {
          ++iv;
          ++n;
        }  
      }
      return n;
    }

    inline size_t size() const { 
      // because we don't know in advance the true size of the subset,
      // we send back the maximum potential size (which is the size of
      // the dtime vector):
      return vi.size(); 
    }
   
    template<typename T>
    inline void subset(Vector<T>& rv, const Vector<T>& v) const { 
      if (vi.size() != v.size()) {
        std::cout << "vi.size(): " << vi.size() << " v.size(): " << v.size() << std::endl;
        throw range_error("size mismatch between vector to subset and time index");
      }
      idx_type i = 0;
      idx_type val;
      if (getfirst(val, i)) {
        rv.push_back(v[val]);
        while (getnext(val, i)) {
          rv.push_back(v[val]);
        }
      }
    }
   
    template<typename T, typename U>
    void subassign(Vector<T>& rv, 
                   const U& u,
                   idx_type& vj) const { 
      subassign_generic(*this, rv, u, vj);      
    }
   
    template<typename T, typename U>
    inline void subassignScalar(Vector<T>& rv, U u) const { 
      subassignScalar_generic(*this, rv, u);
    }
   
    inline void selectNames(Dname& tonames, const Dname& fromnames) const {
      if (fromnames.names.size() > 0) {
        idx_type ii = 0, iv = 0;
        while (iv < vi.size() && ii < idx.size()) {
          if (!idx[ii].sopen ? vi[iv] < idx[ii].s : vi[iv] <= idx[ii].s) {
            ++iv;
          } else if (!idx[ii].eopen ? vi[iv] > idx[ii].e : vi[iv] >= idx[ii].e) {
            ++ii;
          } else {
            tonames.addafter(fromnames.names[iv++]);
          }  
        }
      } else {
        idx_type nb = size(); // get the number of elements set to true
        tonames = Dname(nb);
      }
    }      
   
  }; // end struct IntervalIndex
  

  /// variant type for the different types of indices that index into
  /// an array. We use Variant rather than a base class with virtual
  /// functions for performance reasons as it is not acceptable to pay
  /// a (virtual) function call for each index/element retrieval. The
  /// downside is that a variant cannot be extended, so we need to
  /// come up with all the index types here. This in particular has
  /// the unfortunate consequence to tie Dtime and Interval classes to
  /// Array.

  using IntIndex    = IntIndex_T<size_t>;
  using IntIndexNeg = IntIndexNeg_T<size_t>;

  struct Index {
    typedef Variant<NullIndex,
                    IntIndex,
                    IntIndexNeg,
                    NameIndex,
                    BoolIndex,
                    DtimeIndex,
                    IntervalIndex
                    > VIdx;

    VIdx idx;

    Index(const NullIndex& i)  : idx(i) { }
    Index(const IntIndex& i)   : idx(i) { }
    Index(const IntIndexNeg& i): idx(i) { }
    Index(const NameIndex& i)  : idx(i) { }
    Index(const BoolIndex& i)  : idx(i) { }
    Index(const DtimeIndex& i) : idx(i) { }
    Index(const IntervalIndex& i) : idx(i) { }

    enum IdxType {
      it_null,
      it_int,
      it_int_neg,
      it_names,
      it_bool,
      it_dtime,
      it_interval
    };

    inline size_t trueSize() const {
      switch (idx.which()) {
      case it_null:
        return get<NullIndex>(idx).trueSize();
      case it_int:
        return get<IntIndex>(idx).trueSize();
      case it_int_neg:
        return get<IntIndexNeg>(idx).trueSize();
      case it_names:
        return get<NameIndex>(idx).trueSize();
      case it_bool:
        return get<BoolIndex>(idx).trueSize();
      case it_dtime:
        return get<DtimeIndex>(idx).trueSize();
      case it_interval:
        return get<IntervalIndex>(idx).trueSize();
      default:
        throw std::range_error("Index::trueSize: unknown index type");
      }
    }

    inline size_t size() const {
      switch (idx.which()) {
      case it_null:
        return get<NullIndex>(idx).size();
      case it_int:
        return get<IntIndex>(idx).size();
      case it_int_neg:
        return get<IntIndexNeg>(idx).size();
      case it_names:
        return get<NameIndex>(idx).size();
      case it_bool:
        return get<BoolIndex>(idx).size();
      case it_dtime:
        return get<DtimeIndex>(idx).size();
      case it_interval:
        return get<IntervalIndex>(idx).size();
      default:
        throw std::range_error("Index::size: unknown index type");
      }
    }

    inline bool getfirst(idx_type& val, idx_type& ii) const {
      switch (idx.which()) {
      case it_null:
        return get<NullIndex>(idx).getfirst(val, ii);
      case it_int:
        return get<IntIndex>(idx).getfirst(val, ii);
      case it_int_neg:
        return get<IntIndexNeg>(idx).getfirst(val, ii);
      case it_names:
        return get<NameIndex>(idx).getfirst(val, ii);
      case it_bool:
        return get<BoolIndex>(idx).getfirst(val, ii);
      case it_dtime:
        return get<DtimeIndex>(idx).getfirst(val, ii);
      case it_interval:
        return get<IntervalIndex>(idx).getfirst(val, ii);
      default:
        throw range_error("Index::getfirst: unknown index type");
      }
    }

    inline bool getnext(idx_type& val, idx_type& ii) const {
      switch (idx.which()) {
      case it_null:
        return get<NullIndex>(idx).getnext(val, ii);
      case it_int:
        return get<IntIndex>(idx).getnext(val, ii);
      case it_int_neg:
        return get<IntIndexNeg>(idx).getnext(val, ii);
      case it_names:
        return get<NameIndex>(idx).getnext(val, ii);
      case it_bool:
        return get<BoolIndex>(idx).getnext(val, ii);
      case it_dtime:
        return get<DtimeIndex>(idx).getnext(val, ii);
      case it_interval:
        return get<IntervalIndex>(idx).getnext(val, ii);
      default:
        throw range_error("Index::getnext: unknown index type");
      }
    }

    template<typename T>
    inline void subset(Vector<T>& rv, const Vector<T>& v) const {
      switch (idx.which()) {
      case it_null:
        get<NullIndex>(idx).subset(rv, v);
        break;
      case it_int:
        get<IntIndex>(idx).subset(rv, v);
        break;
      case it_int_neg:
        get<IntIndexNeg>(idx).subset(rv, v);
        break;
      case it_names:
        get<NameIndex>(idx).subset(rv, v);
        break;
      case it_bool:
        get<BoolIndex>(idx).subset(rv, v);
        break;
      case it_dtime:
        get<DtimeIndex>(idx).subset(rv, v);
        break;
      case it_interval:
        get<IntervalIndex>(idx).subset(rv, v);
        break;
      default:
        throw range_error("Index::subset: unknown index type");
      }     
    }

    template<typename T>
    inline void subset(Vector<T>& rv, const Vector<T>& v, 
                       idx_type& ii, idx_type& iv, idx_type pos) const {
      switch (idx.which()) {
      case it_null:
        get<NullIndex>(idx).subset(rv, v);
        break;
      case it_int:
        get<IntIndex>(idx).subset(rv, v, ii, pos);
        break;
      case it_int_neg:
        get<IntIndexNeg>(idx).subset(rv, v, ii, iv, pos);
        break;
      case it_bool:
        get<BoolIndex>(idx).subset(rv, v, ii);
        break;
      case it_names:
      case it_interval:
      case it_dtime:
        throw range_error("can't do a vector subset with this index type");
      default:
        throw range_error("Index::subset: unknown index type");
      }     
    }

    template<typename T, typename U>
    void subassign(Vector<T>& rv, 
                   const U& v,
                   idx_type& vj) const {
      switch (idx.which()) {
      case it_null:
        get<NullIndex>(idx).subassign(rv, v, vj);
        break;
      case it_int:
        get<IntIndex>(idx).subassign(rv, v, vj);
        break;
      case it_int_neg:
        get<IntIndexNeg>(idx).subassign(rv, v, vj);
        break;
      case it_names:
        get<NameIndex>(idx).subassign(rv, v, vj);
        break;
      case it_bool:
        get<BoolIndex>(idx).subassign(rv, v, vj);
        break;
      case it_dtime:
        get<DtimeIndex>(idx).subassign(rv, v, vj);
        break;
      case it_interval:
        get<IntervalIndex>(idx).subassign(rv, v, vj);
        break;
      default:
        throw range_error("Index::subassign: unknown index type");
      }
    }

    template<typename T, typename U>
    void subassignScalar(Vector<T>& rv, U u) const { 
      switch (idx.which()) {
      case it_null:
        get<NullIndex>(idx).subassignScalar(rv, u);
        break;
      case it_int:
        get<IntIndex>(idx).subassignScalar(rv, u);
        break;
      case it_int_neg:
        get<IntIndexNeg>(idx).subassignScalar(rv, u);
        break;
      case it_names:
        get<NameIndex>(idx).subassignScalar(rv, u);
        break;
      case it_bool:
        get<BoolIndex>(idx).subassignScalar(rv, u);
        break;
      case it_dtime:
        get<DtimeIndex>(idx).subassignScalar(rv, u);
        break;
      case it_interval:
        get<IntervalIndex>(idx).subassignScalar(rv, u);
        break;
      default:
        throw range_error("Index::subassignScalar: unknown index type");
      }     
    }
 
    inline void selectNames(Dname& tonames, const Dname& fromnames) const {
      switch (idx.which()) {
      case it_null: 
        get<NullIndex>(idx).selectNames(tonames, fromnames);
        break;
      case it_int:
        get<IntIndex>(idx).selectNames(tonames, fromnames);
        break;
      case it_int_neg:
        get<IntIndexNeg>(idx).selectNames(tonames, fromnames);
        break;
      case it_names:
        get<NameIndex>(idx).selectNames(tonames, fromnames);
        break;
      case it_bool:
        get<BoolIndex>(idx).selectNames(tonames, fromnames);
        break;
      case it_dtime:
        get<DtimeIndex>(idx).selectNames(tonames, fromnames);
        break;
      case it_interval:
        get<IntervalIndex>(idx).selectNames(tonames, fromnames);
        break;
      default:
        throw range_error("Index::selectNames: unknown index type");
      }     
    }

    static inline bool getfirstcol(idx_type& col, 
                                   vector<idx_type>& val,
                                   vector<idx_type>& pi, 
                                   const vector<Index>& i, 
                                   const Vector<idx_type>& dim)
    {
      // get the first index:
      for (idx_type pd = 1; pd<dim.size(); ++pd) {
        if (!i[pd].getfirst(val[pd], pi[pd])) {
          return false;
        }
      }

      // figure out the column at which it is:
      col = 0;
      idx_type mul = 1;
      for (idx_type k=1; k<dim.size(); ++k) {
        if (val[k] >= dim[k]) {
          throw range_error("subscript out of bounds");
        }
        col += val[k]*mul;
        mul *= dim[k];
      }   

      return true;
    }

    static inline bool getnextcol(idx_type& col, 
                                  vector<idx_type>& val,
                                  vector<idx_type>& pi, 
                                  const vector<Index>& i, 
                                  const Vector<idx_type>& dim) 
    {
      // get the next index:
      idx_type pd = 1;
      while (pd < dim.size() && !i[pd].getnext(val[pd], pi[pd])) {
        ++pd;
      }
      if (pd >= dim.size()) {
        return false;
      }
     
      // figure out the column:
      col = 0;
      idx_type mul = 1;
      for (idx_type k=1; k<dim.size(); ++k) {
        if (val[k] >= dim[k]) {
          throw range_error("subscript out of bounds");
        }
        col += val[k]*mul;
        mul *= dim[k];
      }
      return true;
    }

  };
    

  template<typename I, typename T, typename U>
  void subassign_generic(const I& idx, 
                         Vector<T>& rv, 
                         const U& u,
                         idx_type& vj) {
    // find first j:
    idx_type ii = 0, j = 0, nextj = 0;
    std::cout << "subassign_generic entry j: " << j << ", vj: " << vj << std::endl;
    std::cout << "u.size(): " << u.size() << std::endl;
    if (!idx.getfirst(j, ii)) return;
    nextj = j;                // start search for next from j

    // loop finding nextj:
    while (idx.getnext(nextj, ii)) {
      std::cout << "we found next and nextj: " << nextj << ", j: " << j << ", ii: " << ii << std::endl;
      if (nextj - j <= 1)     setv_checkbefore(rv, j, convert<T,typename U::value_type>(u[vj++]));
      else if (nextj - j > 1) setv(rv, j, convert<T,typename U::value_type>(u[vj++]));
      j = nextj;
    }

    // finish the last one:
    std::cout << "last j: " << j << ", vj: " << vj << std::endl;
    setv(rv, j, convert<T,typename U::value_type>(u[vj++]));
    std::cout << "vj is: " << vj << std::endl;
  }


  template<typename I, typename T, typename U>
  inline void subassignScalar_generic(const I& idx, Vector<T>& rv, U u) { 
    // find first j:
    idx_type ii = 0, j = 0, nextj = 0;
    if (!idx.getfirst(j, ii)) return;
    nextj = j;                // start search for next from j

    // loop finding nextj:
    while (idx.getnext(nextj, ii)) {
      if (nextj - j <= 1)     setv_checkbefore(rv, j, convert<T,U>(u));
      else if (nextj - j > 1) setv(rv, j, convert<T,U>(u));
      j = nextj;
    }

    // finish the last one:
    setv(rv, j, convert<T,U>(u));
  }


}  

#endif
