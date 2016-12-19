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


#ifndef ZTS_HPP
#define ZTS_HPP


#include <string>
#include <vector>
#include <memory>
#include "globals.hpp"
#include "array.hpp"
#include "timezone/ztime_vector.hpp"


// namespace zcore { }

namespace arr {
  
  struct zts {

    zts(rsv_t, 
        const Vector<idx_type>& dim_p,
        const std::vector<Vector<zstring>> cnames=std::vector<Vector<zstring>>());

    zts(const Vector<idx_type>& dim_p, 
        const Array<Global::dtime>& tidx_p,
        const Array<double>& v_p, 
        const std::vector<Vector<zstring>> cnames=std::vector<Vector<zstring>>(),
        std::unique_ptr<AllocFactory>&& allocf_a=std::make_unique<FlexAllocFactory>(),
        std::unique_ptr<AllocFactory>&& allocf_idx=std::make_unique<FlexAllocFactory>());
    
    zts(const Vector<idx_type>& dim_p, 
        const Vector<Global::dtime>& tidx_p,
        const Vector<double>& v_p, 
        const std::vector<Vector<zstring>> cnames=std::vector<Vector<zstring>>(),
        std::unique_ptr<AllocFactory>&& allocf_a=std::make_unique<FlexAllocFactory>(),
        std::unique_ptr<AllocFactory>&& allocf_idx=std::make_unique<FlexAllocFactory>());

    zts(Array<Global::dtime> tidx_p, 
        Array<double> a_p,
        std::unique_ptr<AllocFactory>&& allocf_a=std::make_unique<FlexAllocFactory>(),
        std::unique_ptr<AllocFactory>&& allocf_idx=std::make_unique<FlexAllocFactory>()); // right ref here? LLL
    
    zts(std::unique_ptr<AllocFactory>&& allocf_a,
        std::unique_ptr<AllocFactory>&& allocf_idx); 

    zts(const zts& z,
        std::unique_ptr<AllocFactory>&& allocf_a=std::make_unique<FlexAllocFactory>(),
        std::unique_ptr<AllocFactory>&& allocf_idx=std::make_unique<FlexAllocFactory>());

    zts(zts&& z);

    const Array<double>& getArray() const { return *a; }
    const Array<Global::dtime>& getIndex() const { return *idx; }

    /// Get the underlying shared_ptr to the index. This should be
    /// used only in very few and specific cases. One in particular is
    /// making a copy on write pointer to the index.
    std::shared_ptr<Array<double>>& getArrayPtr() const { return a; }
    /// Get the underlying shared_ptr to the array. This should be
    /// used only in very few and specific cases. One in particular is
    /// making a copy on write pointer to the array.
    std::shared_ptr<Array<Global::dtime>>& getIndexPtr() const { return idx; }

    inline bool operator==(const zts& o) const {
      return *idx == *o.idx && *a == *o.a; 
    }

    inline const Vector<idx_type>& getdim() const { return a->getdim(); }
    inline const idx_type getdim(idx_type d) const { return a->getdim(d); }
    inline const idx_type size() const { return a->size(); }
    inline const Vector<double>& getcol(idx_type i) const { return a->getcol(i); }
    inline fsys::path getAllocfDirname() const { return a->allocf->getDirname(); }
    inline void msync(bool async) const { a->msync(async); idx->msync(async); }
    
    zts& append(const char* buf, size_t buflen, size_t& offset);
    zts& appendVector(const char* buf, size_t buflen);

    buflen_pair to_buffer(size_t offset=0) const;
      
    inline zts& addprefix(const string& prefix, idx_type d) { a->addprefix(prefix, d); return *this; }

    zts& resize(idx_type d, idx_type sz, idx_type from=0);

    // Indexing operations (subsetting, subassign) ------------------
    
    /// Complex subsetting.
    template<typename INDEX>
    zts operator()(const vector<INDEX>& i, bool drop=true) const {
      if (i.size() != a->dim.size()) {
        throw range_error("incorrect number of dimensions");
      }
      // the following is a bit more complicated than using
      // 'idx(vector<INDEX>{i[0]})' but in compensation it avoid the
      // copy of 'idx[0]':
      Vector<Global::dtime> sidx;
      i[0].subset(sidx, idx->getcol(0));
      arr::idx_type dropfirst = 2; // protect against vectorification
      return zts(arr::Array<Global::dtime>({sidx.size()}, sidx), (*a)(i, drop, dropfirst));
    }

    /// subassign an array.
    template<typename INDEX, typename U>
    zts& operator()(const vector<INDEX>& i, const Array<U>& u) {
      (*a)(i, u);
      return *this;
    }

    /// subassign a scalar.
    template<typename INDEX, typename U>
    zts& operator()(const vector<INDEX>& i, U u) {
      (*a)(i, u);
      return *this;
    }

    /// subset with a row range. The dummy argument allows templating
    /// together with array::subsetRows.
    zts subsetRows(idx_type from, idx_type to, bool dummy=false) const;

    /// Apply a function on every element of the array.
    template<class F>
    zts& applyf(F f) {
      a->applyf(f);
      return *this;
    } 

    // bind functions ---------
    zts& abind(const zts& z, idx_type d, const string& prefix="");
    zts& abind(const Array<double>& u, idx_type d, const string& prefix="");

  private:
    // can we please get rid of the mutable here? LLL
    mutable std::shared_ptr<Array<double>> a;
    mutable std::shared_ptr<Array<Global::dtime>> idx;
  };
    
  
  /// alignment functions:
  template <typename DS, typename DE>
  zts align_closest(const zts& ts, 
                    const Array<Global::dtime>& y, 
                    const DS& start, 
                    const DE& end) 
  {
    auto dim = ts.getdim();
    setv(dim, 0, y.size());
    Array<double> a(arr::rsv, dim);
  
    for (size_t i=0; i<a.ncols(); ++i) {
      arr::align_closest<double, Global::NANF, DS, DE>
        (ts.getIndex().getcol(0), 
         y.getcol(0), 
         ts.getArray().getcol(i), 
         a.getcol(i), 
         start, 
         end);
    }
  
    return arr::zts(y, std::move(a)); // LLL verify no copy
  }

  /// align_func

  template <typename F, typename DS, typename DE>
  zts align_func(const zts& ts, 
                 const Array<Global::dtime>& y, 
                 const DS& start, 
                 const DE& end) 
  {
    auto dim = ts.getArray().getdim();
    setv(dim, 0, y.size());
    Array<double> a(arr::rsv, dim);
  
    for (size_t i=0; i<a.ncols(); ++i) {
      arr::align_func<double, F, DS, DE>
        (ts.getIndex().getcol(0), 
         y.getcol(0), 
         ts.getArray().getcol(i), 
         a.getcol(i), 
         start, 
         end);
    }
  
    // avoid the copy of y/a here! LLL
    return arr::zts(y, std::move(a));    
  }

  struct LengthMismatch : std::invalid_argument {
    LengthMismatch(const std::string& s) : std::invalid_argument(s) { }
  };
  struct UnorderedIndex : std::invalid_argument {
    UnorderedIndex(const std::string& s) : std::invalid_argument(s) { }
  };
  
} // end namespace zts


#endif 
