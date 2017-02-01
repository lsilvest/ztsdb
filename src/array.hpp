// (C) 2016,2017 Leonardo Silvestri
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
// (C) 2015 Leonardo Silvestri


#ifndef ARRAY_H
#define ARRAY_H

#include <vector>
#include <sstream>
#include <functional>
#include <algorithm> 
#include <numeric>
#include <exception> 
#include <iostream>
#include <set>
#include <utility>
#include <memory>
#include <sys/types.h>
#include "juice/variant.hpp"
#include "allocator_factory.hpp"
#include "vector.hpp"
#include "string.hpp"
#include "misc.hpp"
#include "globals.hpp"
#include "base_types.hpp"
#include "dname.hpp"


//#define DEBUG_COPY


using namespace std;
using namespace Juice;
using namespace std::string_literals;


/// Contains the classes and functions that implement a multidimentional array type.
namespace arr {

  // to disambiguate sequence constructors:
  // call them _tag as in the STL LLL
  struct seq_to_t { };
  struct seq_n_t { };
  constexpr seq_to_t seq_to { };
  constexpr seq_n_t seq_n { };
  
  struct convert_cons_t { };
  constexpr convert_cons_t convert_cons {};

  typedef uint64_t idx_type;

  // a few miscellaneous helper functions:

  /// Check that all parameters have the same size. All parameters
  /// must implement the function 'size'.
  template <typename V>
  inline bool checksize(const V& v) { return true; }
  template <typename V1, typename V2>
  inline bool checksize(const V1& v1, const V2& v2) { return v1.size() == v2.size(); }
  template <typename V1, typename V2, typename... U>
  inline bool checksize(const V1& v1, const V2& v2, const U&... u) {
    return checksize<V1,V2>(v1, v2) && checksize<V1, U...>(v1, u...);
  }

  /// Check that all paramters have compatible dimensions for column
  /// by column manipulation. The function is correct in presence of
  /// various dimensions of size one. Parameters must implement
  /// functions 'ncols' and 'size'.
  template <typename V>
  inline bool checkdims(const V& v) { return true; }
  template <typename V1, typename V2>
  inline bool checkdims(const V1& v1, const V2& v2) { 
    return v1.ncols() == v2.ncols() && v1.size() == v2.size(); }
  template <typename V1, typename V2, typename... U>
  inline bool checkdims(const V1& v1, const V2& v2, const U&... u) {
    return checkdims<V1,V2>(v1, v2) && checkdims<V1, U...>(v1, u...);
  }

  /// Create a directory if dirname is not "".  
  void createDir(const zstring& dirname); // throws

  /// Create the name vectors using the mmap 'dirname' directory. If
  /// 'dirname' is "", uses a standard allocator rather than a mmap
  /// one.
  void buildNames(vector<unique_ptr<Dname>>& names,
                  const Vector<idx_type> dim, 
                  const AllocFactory& allocf,
                  const vector<Vector<zstring>> names_p={Vector<zstring>()}); // throws
  

  // --------------------------------------------------------------
  /// Defines an array type. It is multidimensional, the storage is
  /// one vector per column.
  template<typename T, typename O=std::less<T>>
  struct Array {

    typedef T value_type;
    typedef O comparator;
    typedef Vector<T,O> vector_type;
    
    // constructors -----------------------------------------------

    /// construct a sequence in place, from 't1' to 't2'.
    template<typename U>
    Array(seq_to_t, T t1, T t2, U by,
          std::unique_ptr<AllocFactory>&& allocf_p=std::make_unique<MemAllocFactory>()) 
      : allocf(std::move(allocf_p))
    {
      if (by <= getInitValue<U>()) {
        throw std::out_of_range("'by' parameter must be >= 0");
      }
      auto n = static_cast<idx_type>((t1 < t2 ? t2 - t1 : t1 - t2) / by) + 1;
      dim = Vector<idx_type>({n}, allocf->get("dim"));
      v.emplace_back(make_unique<Vector<T,O>>(noinit_tag, n, allocf->get(0)));

      auto val = t1;
      if (t2 < t1) {
        for (idx_type i=0; i<n; ++i) {
          setv_nocheck(*v[0], i, val);
          val = val - by;
        }  
      } else {
        for (idx_type i=0; i<n; ++i) {
          setv_nocheck(*v[0], i, val);
          val = val + by;
        }
      }
      v[0]->setOrdered(n <= 1 || O()(t1, t2));
      buildNames(names, dim, *allocf);
    }
    
    /// construct a sequence in place, from 't1', with 'n' elements.
    template<typename U>
    Array(seq_n_t, T t1, U by, idx_type n, 
          std::unique_ptr<AllocFactory>&& allocf_p=std::make_unique<MemAllocFactory>()) 
      : allocf(std::move(allocf_p))
    {
      dim = Vector<idx_type>({n}, allocf->get("dim"));
      v.emplace_back(make_unique<Vector<T,O>>(rsv, n, allocf->get(0)));
      v[0]->push_back(t1);
      for (idx_type i=1; i<n; ++i) {
        v[0]->push_back((*v[0])[i-1] + by);
      }  
      buildNames(names, dim, *allocf);
    }

    /// construct array of given dimension, without initializing content.
    /// this is bad as the Array is left in an incoherent state LLL
    template<typename INIT_TYPE_TAG>
    Array(INIT_TYPE_TAG init,
          const Vector<idx_type> dim_p, 
          const vector<Vector<zstring>> names_p=vector<Vector<zstring>>(),
          std::unique_ptr<AllocFactory>&& allocf_p=std::make_unique<MemAllocFactory>())
      : dim(dim_p), allocf(std::move(allocf_p))
    { 
      dim = Vector<idx_type>(dim_p, allocf->get("dim"));
      // if (dim_p.size() == 0) {
      //   throw range_error("'dims' cannot be of length 0");
      // }
      if (dim.size()) {         // only initialize if this is not the null array
        auto ncols = accumulate(dim.begin()+1, dim.end(), 1.0, std::multiplies<idx_type>());
        // check each name subvector x; it has to be either 0 or of the corresponding dim[x]
        v.resize(ncols);
        for (idx_type n=0; n<ncols; ++n) {
          v[n] = make_unique<Vector<T,O>>(init, dim[0], allocf->get(n));
        }
        buildNames(names, dim_p, *allocf, names_p);
      }
    }
    
    
    /// Construct an array from file. Note that the data must have
    /// been tested to determine typename 'T'. 'mapped' indicates if
    /// the array is to remain mmapped to the file or if it should
    /// simply be read in memory (not implemented yet LLL).
    Array(std::unique_ptr<AllocFactory>&& allocf_p)
      : allocf(std::move(allocf_p))
    {
      // grab the dim vector:
      dim = Vector<idx_type>(allocf->get("dim"));
      // grab the data vectors
      auto ncols = dim.size() ? 
        accumulate(dim.begin()+1, dim.end(), 1.0, std::multiplies<idx_type>()) : 0;
      for (idx_type j=0; j<ncols; ++j) {
        v.emplace_back(make_unique<Vector<T,O>>(allocf->get(j)));
      }
      // grab the name vector:
      for (idx_type j=0; j<dim.size(); ++j) {
        names.emplace_back
          (make_unique<Dname>(dim[j], Vector<zstring>(allocf->get("names" + std::to_string(j)))));
      }
    }


    Array(const Vector<idx_type> dim_p, 
          const Array<T>& v_p, 
          const vector<Vector<zstring>> names_p=vector<Vector<zstring>>(),
          std::unique_ptr<AllocFactory>&& allocf_p=std::make_unique<MemAllocFactory>())
      : allocf(std::move(allocf_p))
    {
      dim = Vector<idx_type>(dim_p, allocf->get("dim"));
      auto ncols = dim.size() ? 
        accumulate(dim.begin()+1, dim.end(), 1.0, std::multiplies<idx_type>()) : 0;
      // data initialization from v_p:
      if (v_p.size() == 0) {
        for (idx_type n=0; n<ncols; ++n) {
          v.emplace_back(make_unique<Vector<T,O>>(rsv, dim[0], allocf->get(n)));
        }
      }
      else if (v_p.size() == 1) {
        for (idx_type n=0; n<ncols; ++n) {
          v.emplace_back(make_unique<Vector<T,O>>(dim[0], v_p[0], allocf->get(n)));
        }
      } else {
        if (v_p.getdim(0) == dim[0] && ncols == v_p.v.size()) {
          // same dimensions, we can take advantage of that:
          for (idx_type n=0;  n<ncols; ++n) {
            v.emplace_back(make_unique<Vector<T,O>>(*v_p.v[n], allocf->get(n)));
          }
        }
        else {
          if (v_p.size() != ncols * dim[0]) {
            throw range_error("data does not fit array dimensions");
          }
          // different dimensions, copy elements one by one:
          idx_type idx=0;
          for (idx_type n=0;  n<ncols; ++n) {
            v.emplace_back(make_unique<Vector<T,O>>(rsv, dim[0], allocf->get(n)));
            for (idx_type i=0; i<dim[0]; ++i) {
             v.back()->push_back(v_p[idx++]);
            }
          }
        }
      }
      buildNames(names, dim_p, *allocf, names_p);
    }

    /// Main constructor with, as parameters, a vector of values, a
    /// dimension vector, column names and potentially a directory
    /// name. If the directory name is specified, the array will be
    /// memory mapped.
    Array(const Vector<idx_type> dim_p, 
          const Vector<T,O>& v_p, 
          const vector<Vector<zstring>> names_p=vector<Vector<zstring>>(),
          std::unique_ptr<AllocFactory>&& allocf_p=std::make_unique<MemAllocFactory>()) 
      : allocf(std::move(allocf_p))
    {  
      //      cout << "begin array constructor" << endl;
      // if (dim_p.size() == 0) {
      //   throw range_error("'dims' cannot be of length 0");
      // }
      dim = Vector<idx_type>(dim_p, allocf->get("dim"));
      auto ncols = dim.size() ? 
        accumulate(dim.begin()+1, dim.end(), 1.0, std::multiplies<idx_type>()) : 0;
      // check each name subvector x; it has to be either 0 or of the corresponding dim[x]
      if (v_p.size() == 0) {
        // in R:
        // array(vector(mode="numeric"), c(2,2))
        //      [,1] [,2]
        // [1,]   NA   NA
        // [2,]   NA   NA
        // only if dim=0 is v_p.size() allowed to be 0
        // but I think we want to do away with vectors, so this case might never arise...
        for (idx_type n=0; n<ncols; ++n) {
          v.emplace_back(make_unique<Vector<T,O>>(dim[0], getInitValue<T>(), allocf->get(n)));
        }
      }
      else if (v_p.size() == 1) {
        for (idx_type n=0; n<ncols; ++n) {
          v.emplace_back(make_unique<Vector<T,O>>(dim[0], v_p[0], allocf->get(n)));
        }
      } else {
        if (v_p.size() != ncols * dim[0]) {
          throw range_error("data does not fit array dimensions");
        }
        idx_type offset = 0;
        for (idx_type n=0; n<ncols; ++n) {
          v.emplace_back(make_unique<Vector<T,O>>(v_p.begin() + offset, 
                                                  v_p.begin() + offset + dim[0], 
                                                  allocf->get(n)));
          offset += dim[0];
        }
      }
      buildNames(names, dim_p, *allocf, names_p);
    }

    /// nearly a copy constructor!
    /// need to figure out which allocator to use here LLL
    Array(const Vector<idx_type> dim_p, 
          const vector<unique_ptr<Vector<T,O>>>& v_p, 
          const vector<unique_ptr<Dname>>& names_p,
          std::unique_ptr<AllocFactory>&& allocf_p=std::make_unique<MemAllocFactory>())
      : dim(dim_p), allocf(std::move(allocf_p))
    { 
#ifdef DEBUG_COPY
      if (dim_p.size() == 1) {
        cout << "nearly copying array " << dim_p[0] << endl;
      }
      else {
        cout << "nearly copying array " << dim_p[0] << "x" << dim_p[1] << endl;
      }      
#endif
      for (auto& e : v_p) {
        v.emplace_back(make_unique<Vector<T,O>>(*e));
      }
      for (auto& e : names_p) {
        names.emplace_back(make_unique<Dname>(*e));
      }
    }

    // copy constructors:
    Array(const Array& u, 
          std::unique_ptr<AllocFactory>&& allocf_p=std::make_unique<MemAllocFactory>())
      : dim(u.dim), allocf(std::move(allocf_p)) {
#ifdef DEBUG_COPY
      if (u.dim.size() == 1) {
        cout << "copying array " << u.dim[0] << endl;
      }
      else {
        cout << "copying array " << u.dim[0] << "x" << u.dim[1] << endl;
      }
#endif
      // for each vector, make a unique_ptr point to a copy:
      v.reserve(u.v.size());
      for (idx_type i=0; i<u.v.size(); ++i) {
        v.emplace_back(make_unique<Vector<T,O>>(Vector<T,O>(*u.v[i]), allocf->get(i))); 
      }
      for (idx_type i=0; i<u.names.size(); ++i) {
        names.emplace_back
          (make_unique<Dname>(dim[i], 
                              Vector<zstring>(u.names[i]->names,
                                              allocf->get("names" + std::to_string(i)))));
      }
    }

    /// Convert to another array type.
    template<typename U, typename OU=std::less<U>>
    Array(convert_cons_t, const Array<U,OU>& u,
          std::unique_ptr<AllocFactory>&& allocf_p=std::make_unique<MemAllocFactory>())
      : dim(u.dim), allocf(std::move(allocf_p))
    {
      v.reserve(u.v.size());
      for (idx_type n=0; n<u.v.size(); ++n) {
        v.emplace_back(make_unique<Vector<T,O>>(rsv, dim[0]));
        for (idx_type r=0; r<dim[0]; ++r) {
          v[n]->push_back(convert<T,U>((*u.v[n])[r]));
        }
      }
      for (auto& e : u.names) {
        names.emplace_back(make_unique<Dname>(Dname(*e))); 
      }
    }

    Array(Array&& u) { 
#ifdef DEBUG_COPY
      if (u.dim.size() == 1) {
        cout << "swapping array on universal reference " << u.dim[0] << endl;
      }
      else {
        cout << "swapping array on universal reference " << u.dim[0] << "x" << u.dim[1] << endl;
      }
#endif
      swap(u);
    }

    // template<typename U, class F>
    // Array<T>(const Array<U>& u, F f) : dim(u.dim)
    // {
    //   v.reserve(u.v.size());
    //   for (idx_type n=0; n<u.v.size(); ++n) {
    //     v.emplace_back(make_unique<Vector<T,O>>(dim[0]));
    //     for (idx_type r=0; r<dim[0]; ++r) {
    //       (*v[n])[r] = f((*u.v[n])[r]);
    //     }
    //   }
    //   for (auto& e : u.names) {
    //     names.emplace_back(make_unique<Dname>(Dname(*e))); 
    //   }
    // }

    template<typename U, class G>
    Array(const Array<U>& u, G g, bool abba,
          std::unique_ptr<AllocFactory>&& allocf_p=std::make_unique<MemAllocFactory>())
      : dim(u.dim) // LLL use a type to discriminate, not a dummy arg
    {
      v.reserve(u.v.size());
      for (idx_type n=0; n<u.v.size(); ++n) {
        v.emplace_back(make_unique<Vector<T,O>>());
        *v[n] = g(*u.v[n]);
      }
      for (auto& e : u.names) {
        names.emplace_back(make_unique<Dname>(Dname(*e))); 
      }
    }

    Array(Vector<T,O>&& vv) : dim({vv.size()}) {
      v.emplace_back(make_unique<Vector<T,O>>(vv));
      names.emplace_back(make_unique<Dname>(vv.size())); 
    }

    virtual ~Array() { }

    // assignment operator this won't work with the allocf LLL
    Array& operator=(Array u) {
#ifdef DEBUG_COPY
      if (u.dim.size() == 1) {      
        cout << "array assignment "  << u.dim[0] << endl;
      }
      else {
        cout << "array assignment "  << u.dim[0] << "x" << u.dim[1] << endl;
      }
#endif
      return swap(u);
    }

    Array& swap(Array& u) {
      std::swap(v, u.v);
      std::swap(dim, u.dim);
      std::swap(names, u.names);
      std::swap(allocf, u.allocf);
      return *this;
    }

    /// equality operator. We require that both the content and the
    /// names be equal.
    bool operator==(const Array<T>& u) const {
      if (dim == u.dim) {
        for (idx_type j=0; j<u.names.size(); ++j) {
          if (*names[j] != *u.names[j]) {
            return false;
          }
        }
        for (idx_type j=0; j<u.v.size(); ++j) {
          if (*v[j] != *u.v[j]) {
            return false;
          }
        }
        return true;
      }
      else {
        return false;
      }
    }

    /// address the Array as if it were a column vector
    // inline typename vector<T>::reference operator[](idx_type i) { 
    //   if (dim[0] == 0) {
    //     throw range_error("subscript out of bounds");
    //   }
    //   idx_type col = i / dim[0];
    //   idx_type off = i % dim[0];
    //   if (col >= v.size() || off >= dim[0]) {
    //     throw range_error("subscript out of bounds");
    //   }
    //   return (*v[col])[off];
    // }

    /// address the Array as if it were a column vector
    inline T operator[](idx_type i) const { 
      if (size() == 0) {
        throw range_error("subscript out of bounds");
      }
      idx_type col = i / dim[0];
      idx_type off = i % dim[0];

      if (col >= v.size() || off >= dim[0]) {
        throw range_error("subscript out of bounds");
      }
      return (*v[col])[off]; 
    }


    // to buffer ------------------

    /// Allocate and serialize the array to a buffer. Note that only
    /// the data gets copied out - column names are ignored.

    size_t getBufferSize() const {
      return dim.getBufferSize() + v.size() * v[0]->getBufferSize();
    }

    size_t to_buffer(char* buf) const {
      // what if we have null vectors... LLL
      // figure out the size we need to allocate:
      size_t offset = 0;
      offset += dim.to_buffer(buf);
      for (auto& colptr : v) {
        offset += colptr->to_buffer(buf + offset);
      }
      return offset;
    }

    // Indexing operations (subsetting, subassign) ------------------
   
    /// Complex subsetting.

    /// Subset a vector or a multidimentional array as a vector with
    /// one index. This function does not handle dimnames. When the
    /// dimnames are desired for a vector, the operator that takes a
    /// vector of index will do the correct job. For multidimential
    /// array dimnames cannot be preserved.
    template<typename INDEX>
    Array<T> operator()(const INDEX& i) const {
      Array<T> r(rsv, {0});

      idx_type ii = 0;
      idx_type iv = 0;
      idx_type pos = 0;
      for (idx_type k=0; k<v.size(); ++k)  {
        i.subset(*r.v[0], *v[k], ii, iv, pos);
        pos += dim[0];
      }

      arr::setv(r.dim, 0, r.v[0]->size());
      r.names[0]->resize(r.v[0]->size());
      return r;
    }

    template<typename INDEX>
    Array operator()(const vector<INDEX>& i, bool drop=true, idx_type dropfirst=0) const {
      if (i.size() == 1 && dim.size() > 1) {
        return (*this)(i[0]);
      }
      if (i.size() != dim.size()) {
        throw std::range_error("incorrect number of dimensions");
      }

      // figure out the end size of the array
      Vector<idx_type> rdim(i.size());
      auto transfop = [](const INDEX& i) { return i.size(); };
      std::transform(i.begin(), i.end(), rdim.begin(), transfop);
      auto rdimUndropped = rdim;
      if (drop && dropfirst<rdim.size()) { // remove all dimensions of size 1:
        rdim.erase(std::remove_if(rdim.begin()+dropfirst, 
                                  rdim.end(), 
                                  [](idx_type i) { return i == 1; }),
                   rdim.end());
        if (!rdim.size()) {
          rdim.push_back(1);
        }
      }

      idx_type rncols = rdim.size() == 1 ? 1 :
        accumulate(rdim.begin()+1, rdim.end(), 1.0, std::multiplies<idx_type>());
      auto pi = vector<idx_type>(dim.size(), 0);  // keep track of the current index
      auto val = vector<idx_type>(dim.size(), 0); // keep track of the value of the index
      auto rv = vector<unique_ptr<Vector<T,O>>>();
      rv.reserve(rncols);
      for (idx_type j=0; j<rncols; ++j) {
        rv.emplace_back(make_unique<Vector<T,O>>());
      }

      // We have two cases here; the special case which is when we
      // drop the first dim. In that case we have one unique index
      // 'ui' in dimension 0 and the end result is composed of
      // extracting for all columns at index 'ui' and with these
      // forming the resulting subsetted array:
      if (drop && dropfirst==0 && rdimUndropped[0] == 1) {
        idx_type col = 0;
        idx_type uidx, pidx;
        auto r = i[0].getfirst(uidx, pidx);
        if (!r) {
          // might just have a 0 dimension here, so treat like false false LLL
          throw std::range_error("failed getfirst on dim[0] = 1");
        }
        if (uidx >= dim[0]) {
          throw range_error("subscript out of bounds");
        }            
        bool res = INDEX::getfirstcol(col, val, pi, i, dim);
        idx_type rcol = 0;
        while (res) {
          if (col >= v.size()) {
            throw range_error("subscript out of bounds");
          }            
          rv[rcol]->push_back((*v[col])[uidx]);
          if (rv[rcol]->size() == rdim[0]) {
            ++rcol;
          }
          res = INDEX::getnextcol(col, val, pi, i, dim);
        }
      } 
      // and here is the usual case where the first dimension is not
      // dropped and we just subset each column; the result of the
      // subset is of course stored in rv (our result vector of
      // vector):
      else {
        idx_type col = 0;
        bool res = INDEX::getfirstcol(col, val, pi, i, dim);
        idx_type rcol = 0;
        while (res) {
          i[0].subset(*rv[rcol], *v[col]);
          res = INDEX::getnextcol(col, val, pi, i, dim);
          ++rcol;
        }
        
        // for indices that do not know the correct end size, we check
        // again to either resize or reorganize in case of a drop LLL
        if (rv.size() && rv[0]->size() != rdim[0]) {
          if (drop && dropfirst==0 && rv[0]->size() == 1 && rv.size()!=1) {
            throw range_error("resize first dim to 1 not implemented");
          }
          else {
            arr::setv(rdim, 0, rv[0]->size());
          }
        }
      }

      // handle the copying of names:
      auto rnames = vector<unique_ptr<Dname>>();
      idx_type rnameIdx = 0;
      for (idx_type j=0; j<rdimUndropped.size(); ++j) {
        bool scalarResult = rdim.size()==1 && rdim[0]==1;
        // test if the dimension under question is being dropped; conditions for dropping:
        // 1: first iteration (j==0), not a vector, drop, dropfirst==0, dim 0 is 1, not scalar
        bool cond1 = j==0 && drop && dropfirst==0 && rdimUndropped[0]==1 && !scalarResult;
        // 2: other iterations (j>0), drop, dropfirst <= j, dim under question is 1
        bool cond2 = j>0 && drop && dropfirst <= j && rdimUndropped[j]==1;
        // if none of the 2 conditions for dropping is fullfilled, then don't copy dim:
        if (!cond1 && !cond2) {
          rnames.emplace_back(make_unique<Dname>(0));
          if (names[j]->names.size() > 0) {
            i[j].selectNames(*rnames[rnameIdx], *names[j]);
          } 
          else {
            *rnames[rnameIdx] = Dname(rdim[rnameIdx]);
          }
          ++rnameIdx;
        }
      }

      return Array<T>(rdim, rv, rnames);
    }


    // array elt/subset assignment -----

    /// subassign an array.

    template<typename INDEX, typename U>
    Array& operator()(const vector<INDEX>& i, const Array<U>& u) {
      if (i.size() != dim.size()) {
        throw std::range_error("incorrect number of dimensions");
      }
      // check u has same number of elements as i. This will be
      // inefficient for indices that have complex calculations to
      // figure out their size, but we can't really let an incorrect
      // subassign mess up an array:
      idx_type nelts = i[0].trueSize();
      for (idx_type d = 1; d < i.size(); ++d) {
        nelts *= i[d].trueSize();
      }
      if (nelts != u.size()) {
        throw range_error("number of items to replace is not equal to the replacement length");
      }

      // assign a according to i
      auto pi = vector<idx_type>(dim.size(), 0);  // keep track of the current index
      auto val = vector<idx_type>(dim.size(), 0); // keep track of the value of the index
      idx_type col = 0;
      bool res = INDEX::getfirstcol(col, val, pi, i, dim);
      idx_type uj = 0;
      while (res) {
        if (col >= v.size()) {
          throw range_error("subscript out of bounds");
        }           
        i[0].subassign(*v[col], u, uj);
        res = INDEX::getnextcol(col, val, pi, i, dim);
      }
      return *this;
    }

    /// subassign a scalar.
    template<typename INDEX, typename U>
    Array& operator()(const vector<INDEX>& i, U u) {
      if (i.size() != dim.size()) {
        throw std::range_error("incorrect number of dimensions");
      }

      // assign a according to i
      auto pi = vector<idx_type>(dim.size(), 0);  // keep track of the current index
      auto val = vector<idx_type>(dim.size(), 0); // keep track of the value of the index
      idx_type col = 0;
      bool res = INDEX::getfirstcol(col, val, pi, i, dim);
      // idx_type ucol = 0;
      // can use the following (should be faster) if we know i.dim is the same a u.dim:
      // while (res) {
      //   docopyIndexResult(v[col], u.v[ucol++], i[0]);
      //   res = getnextcol(col, val, pi, i, dim);
      // }
      while (res) {
        if (col >= v.size()) {
          throw range_error("subscript out of bounds");
        }            
        i[0].subassignScalar(*v[col], u);
        res = INDEX::getnextcol(col, val, pi, i, dim);
      }
      return *this;
    }


    T operator[](const vector<idx_type>& i) const {
      if (i.size() != dim.size()) {
	throw range_error("index dimension mismatch");
      }
      if (i[0] >= dim[0]) {
        throw range_error("subscript out of bounds");
      }
      idx_type col = 0;
      idx_type mul = 1;
      for (idx_type k=1; k<dim.size(); ++k) {
	if (i[k] >= dim[k]) {
	  throw range_error("subscript out of bounds");
	}
        col += i[k]*mul;
        mul *= dim[k];
      }
      return (*v[col])[i[0]];
    }


    /// Subset the specified row range:
    Array subsetRows(idx_type n, idx_type from=0, bool addrownums=false) const {
      if (from + n > dim[0]) {
        throw range_error("range out of bounds");
      }
      auto aDim = dim;
      arr::setv(aDim, 0, n);
      Array a(arr::rsv, aDim); 
      for (idx_type i=0; i<ncols(); ++i) {
        for (idx_type j=from; j<from+n; ++j) {
          a.v[i]->push_back((*v[i])[j]);
        }
      }
      // copy dnames:
      // first dimension is the only tricky one:
      if (hasNames(0) || addrownums) {
        for (idx_type j=from; j<from+n; ++j) {
          (*a.names[0]).assign(j-from, hasNames(0) ? (*names[0])[j] : 
                               "[" +std::to_string(j+1)+std::string(dim.size()-1, ',')+"]");
        }
      }
      // copy the other dimension's dnames as is:
      for (idx_type j=1; j<dim.size(); ++j) {
        *a.names[j] = *names[j];        
      }

      return a;
    }


    inline bool hasNames(idx_type d) const {
      if (d >= names.size()) {
        throw out_of_range("name index out of bound");
      }
      return names[d]->hasNames();
    }

    inline const Dname& getNames(idx_type d) const {
      if (d >= names.size()) {
        throw out_of_range("name index out of bound");
      }
      return *names[d];
    }

    inline const Vector<zstring>& getNamesVector(idx_type d) const {
      if (d >= names.size()) {
        throw out_of_range("name index out of bound");
      }
      return names[d]->names;
    }

    inline bool isVector() const {
      return dim.size() == 1;
    }

    inline bool isScalar() const {
      return dim.size() == 1 && dim[0] == 1;
    }

    inline bool isOrdered() const {
      return std::all_of(v.begin(), v.end(), [](const auto& e){ return e->isOrdered(); });
    }

    inline bool isOrdered(idx_type i) const {
      return v[i]->isOrdered();
    }
   
    inline const AllocFactory& getAllocFactory() const { return *allocf.get(); }

    Array& addprefix(const string& prefix, idx_type d) {
      if (prefix.length() > 0) {
        if (d < dim.size()) {
          names[d]->addprefix(prefix);
        }
        else if (d >= dim.size()) {
          // so a vector of 3 might become 3x1 or 3x1x1, etc.
          names.reserve(d+1);
          for (idx_type j=dim.size(); j<d+1; ++j) {
            names.emplace_back(make_unique<Dname>(1));
          }
          dim.resize(d+1, 1);
          names[d]->addprefix(prefix);        
        } 
        else {
          throw range_error("addprefix: dimension out of range");
        }
      }
      return *this;
    }


    /// Get the total number of elements in the array.
    inline idx_type size()  const { return v.size() * nrows(); }
    inline idx_type ncols() const { return v.size(); }
    inline idx_type nrows() const { return dim.size() ? dim[0] : 0; }

    // Get a reference to the underlying column:
    inline const Vector<T,O>& getcol(idx_type i) const { 
      if (i >= v.size()) {
        throw std::out_of_range("getcol: column out of range");
      }
      return *(v[i]); 
    }
    inline Vector<T,O>& getcol(idx_type i) { 
      if (i >= v.size()) {
        throw std::out_of_range("getcol: column out of range");
      }
      return *(v[i].get()); 
    }
    
    /// Get the vector of names in dimention d.
    inline const Dname& getnames(idx_type d) const { 
      if (d >= names.size()) {
        throw std::out_of_range("getnames: dimension out of range");
      }
      return *names[d];  
    }
    inline const Vector<idx_type>& getdim() const { return dim; }
    inline const idx_type getdim(idx_type d) const { 
      if (d >= dim.size()) {
        throw std::out_of_range("getnames: dimension out of range");
      }
      return dim[d]; 
    }

    inline fsys::path getAllocfDirname() const { return allocf->getDirname(); }
    void msync(bool async) const {
      dim.getAllocator()->msync(async);
      for (auto& col : v) {
        col->getAllocator()->msync(async);
      }
      for (auto& nm : names) {
        nm->names.getAllocator()->msync(async);
      }      
    }

    // concat and bind operations -------------------------------------------

    /// Checks if dimensions of vectors 'v' and 'u' are compatible for
    /// a bind in dimension 'i'.
    static inline bool checkdims(const Vector<idx_type>& v, const Vector<idx_type>& u, idx_type i) {
      // a 0 dimentional array can be bound with any other array:
      if (!v.size() || !u.size()) {
        return true;
      }
      idx_type up = min(v.size(), u.size());
      for (idx_type j=0; j<up; ++j) {
        if (j == i) continue;
        if (v[j] != u[j]) {
          return false;
        }
      } 
      // if different number of dims, then they have to be all 1
      for (idx_type j=up; j<v.size(); ++j) {
        if (j == i)    continue;
        if (v[j] != 1) return false;
      } 
      for (idx_type j=up; j<u.size(); ++j) {
        if (j == i)    continue;
        if (u[j] != 1) return false;
      } 

      return true;
    }


    template<typename U>
    Array& concat(const Array<U>& u, const string& prefix="") {
      if (dim.size() > 1 || u.dim.size() > 1) {
        throw range_error("concat can only handle dim==0 and vectors");
      }
      for (auto e: *u.v[0]) {
        v[0]->push_back(convert<T,U>(e));
      }
      auto newnames = *u.names[0];
      if (prefix.length()) {
        newnames.addprefix(prefix);
      }
      names[0]->addafter(newnames);
      setv(dim, 0, dim[0] + u.dim[0]);   
      return *this;  
    }


    /// Concatenate (push_back) one scalar element at the end of a
    /// Array vector. The Array must be a vector.
    template<typename U>
    Array& concat(const U& u, const string& name="") { 
      if (dim.size() == 0) {
        dim.push_back(0);
        names.push_back(make_unique<Dname>(0));
      }
      else if (dim.size() > 1) {
        throw range_error("concat can only handle dim==0 and vectors");
      }
      if (v.size() == 0) {
        v.push_back(make_unique<Vector<T,O>>(dim[0]));
      }

      v[0]->push_back(convert<T,U>(u));
      names[0]->addafter(name);
      arr::setv(dim, 0, dim[0] + 1);
      return *this;  
    }


    /// bind arrays 't' and 'u' in an arbitrary dimension 'd'.
    /// bind rules:
    /// arrays with one of more 0 in a dimension:
    /// b 0x1, 0x1, d1 = 0 x 2; note that names exist, but no data
    /// b 0x0, 0x1, d1 = 0 x 1;  
    /// if there's just one 0 in any dimension, then bind with a
    /// non-empty vector is equivalent (for data) to a copy
    template<typename U>
    Array& abind(const Array<U>& u, idx_type d, const string& prefix="") {
      if (reinterpret_cast<const Array<U>*>(this) == &u) {
        // prevent binding to self!
        throw range_error("cannot bind to self.");
      }

      // if (std::all_of(dim.begin(), dim.end(), [](idx_type i){ return i==0; })) { 
        
      // }
      // For the purpose of rbind, column vectors are always considered
      // to be row vectors. It seems weird, especially when one
      // considers that t(1:3) gives a row vector... But we follow R's
      // convention (albeit with a rather inefficient implementation):
      if (d==0) {
        if (dim.size() == 1) {
          *this = transpose(*this);
          return abind<U>(u.dim.size() == 1 ? transpose(u) : u, d, prefix);
        }
        else if (u.dim.size() == 1) {
          return abind<U>(transpose(u), d, prefix);
        }
      }
      if (!checkdims(dim, u.dim, d)) {
        throw range_error("incorrect dimensions for abind");
      }
      // handle the copying of dim when dim.size()==0, i.e. when we
      // are binding with the null matrix; the dim of u is copied:
      if (dim.size() == 0) {
        for (idx_type j=0; j<u.dim.size(); ++j) {
          dim.push_back(j == d ? 0 : u.dim[j]); // the last part of this function sets dim d
          names.emplace_back(
            make_unique<Dname>(j == d ? 0 : u.dim[j],
                               Vector<zstring>(0, getInitValue<zstring>(),
                                               allocf->get("names" + std::to_string(j)))));
        }
      }
      // potentially need to increase dimensions:
      if (d >= dim.size()) {      
        // we add dimension of size 1 (i.e. a vector of 3 is
        // considered a 3x1 for a cbind) except for the null array,
        // which needs size 0 in dimension d to avoid spurious column
        // creation:
        for (idx_type j=dim.size(); j<d+1; ++j) {
          names.emplace_back(
            make_unique<Dname>(v.size()==0 && j==d ? 0 : 1,
                               Vector<zstring>(0, getInitValue<zstring>(),
                                               allocf->get("names" + std::to_string(j)))));
          dim.push_back(v.size()==0 && j==d ? 0 : 1);
        }
      }
      if (d==0) {
        // not touching columns, just making the rows longer;
        for (idx_type j=0; j<u.v.size(); ++j) {
          if (j >= v.size()) {
            // if the column doesn't exist, create it:
            v.emplace_back(make_unique<Vector<T,O>>(rsv, u.v[j]->size(),
                                                  allocf->get(std::to_string(j))));
          }
          for (auto e: *u.v[j]) {
            v[j]->push_back(convert<T,U>(e));
          }
        }  
      } else {
        // adding u.v.size() columns:
        auto origSize = v.size();
        v.reserve(origSize + u.v.size());
        for (idx_type j=0; j<u.v.size(); ++j) {
          // the following is potentially mmapped if 'dirname' is not "":
          v.emplace_back(make_unique<Vector<T,O>>(rsv, dim[0],
                                                allocf->get(std::to_string(origSize+j))));
        }
        idx_type ucols = d < u.dim.size() ?
                             accumulate(u.dim.begin()+1, u.dim.begin()+d+1, 1, multiplies<idx_type>()) :
          accumulate(u.dim.begin()+1, u.dim.end(), 1, multiplies<idx_type>());
        idx_type cols  = d < dim.size() ?
                             accumulate(dim.begin()+1, dim.begin()+d+1, 1, multiplies<idx_type>()) :
          accumulate(dim.begin()+1, dim.end(), 1, multiplies<idx_type>());
        // how many times we need to do this:
        idx_type n      =  d < dim.size() ?
                               accumulate(dim.begin()+d+1, dim.end(), 1, multiplies<idx_type>()) : 1;    
        idx_type destoff = v.size() - 1;    // to where we need to copy the cols in r
        idx_type srcoff  = origSize - 1;    // from where we need to swap the cols in r
        idx_type uoff    = u.v.size() - 1;  // index in array u
        for (idx_type j=n; j>0; --j) {
          // copy the slice of cols from t to r: 
          bool needSwap = true;
          for (idx_type k=0; k < ucols; ++k) {
            for (idx_type l=0; l<u.v[uoff]->size(); ++l) {
              v[destoff]->push_back(convert<T,U>((*u.v[uoff])[l]));
            } 
            --destoff;
            if (uoff > 0) {
              --uoff;
            } else {
              needSwap = false;
            }
          }
          if (needSwap) {
            // move the columns: 
            for (idx_type k=0; k < cols; ++k) {
              std::swap(v[srcoff--], v[destoff--]);
            }
          }
        }
      }
      // add new names in dimension d if present:
      auto newnames = d < u.dim.size() ? *u.names[d] : Dname(1);
      newnames.addprefix(prefix);
      names[d]->addafter(newnames);
      // add new names in for dimensions other than 'd' if it was missing them:
      for (idx_type i=0; i<names.size(); ++i) {
        if (i==d) continue;
        if (!hasNames(i) && i < u.names.size() && u.hasNames(i)) {
          *names[i] = *u.names[i];
        }
      }
      setv(dim, d, dim[d] + (d < u.dim.size() ? u.dim[d] : 1));   
      return *this;  
    }

    template<typename U>
    Array& rbind(const Array<U>& u) {
      return abind<U>(u, 0); 
    }

    template<typename U>
    Array& cbind(const Array<U>& u) {   
      return abind<U>(u, 1);
    }


    // that's not an offset, it's the nb bytes read LLL
    Array& append(const char* buf, size_t buflen, size_t& offset) {
      auto adim = Vector<idx_type>(const_cast<char*>(buf), buflen);
      
      if (dim.size() == 0) {
        // figure out if we want to support that...
        /// LLL, yes, we do!!!
        throw out_of_range("append on null array not implemented");        
      }
      if (!checkdims(dim, adim, 0)) {
        throw out_of_range("incorrect dimensions for append");
      } 
      
      offset = sizeof(RawVector<idx_type>) + sizeof(idx_type)*adim.size();
      try {
        for (size_t i=0; i<v.size(); ++i) {
          offset += v[i]->append(buf + offset, buflen - offset);
        }
      }
      catch (...) {
        for (size_t i=0; i<v.size(); ++i) {
          v[i]->resize(dim[0]);    // stay in a coherent state!
        }
        throw;
      }
    
      // it's important to do the dimension update at the end, as it
      // allows mmapped arrays to recover in the event of a crash in
      // the middle of an append:
      names[0]->addafter(Dname(adim[0]));
      setv(dim, 0, dim[0] + adim[0]);
      return *this;
    }


    Array& appendVector(const char* buf, size_t buflen) {
      auto data = Vector<T,O>(const_cast<char*>(buf), buflen);
      // the data has to be a multiple of the number of columns:
      if (v.size() == 0) {
        // figure out if we want to support that...
        /// LLL, yes, we do!!!
        throw out_of_range("appendVector on null array not implemented");        
      }
      if (data.size() % v.size()) {
        throw out_of_range("appendVector: incorrect vector size");        
      }
      idx_type nrows = data.size() / v.size();
      idx_type i = 0, j=0;
      for (auto e : data) {
        v[i]->push_back(e);
        if (++j == nrows) { j = 0; ++i; }
      }

      names[0]->addafter(Dname(nrows));
      setv(dim, 0, dim[0] + nrows);
      return *this;
    }


    Array& resize(idx_type d, idx_type sz, idx_type from=0) {
      if (d !=0) {
        throw logic_error("resize only implemented for dim 0");
      }
      if (dim.size() == 0) {
        dim.push_back(0);
        names.push_back(make_unique<Dname>(0));
      }
      if (v.size() == 0) {
        v.push_back(make_unique<Vector<T,O>>(dim[0]));
      }
      
      for (idx_type j=0; j<v.size(); ++j) {
        v[j]->resize(sz, from);
      }
      setv(dim, d, sz);
      names[0]->resize(sz, from);
      return *this;
    }

    // apply operations -------------------------------------------

    /// Apply a function on every element of an array.
    template<class F>
    Array& applyf(F f) {
      for (idx_type n=0; n<v.size(); ++n) {
        for (idx_type i=0; i<v[n]->size(); ++i) {
          setv_checkbefore(*v[n], i, f((*v[n])[i]));
        }
      }
      return *this;
    } 

    /// Apply functor F to any number of arguments. Each argument must
    /// have a 'getcol' function that returns an instance of a class
    /// that is subsettable. In addition the type 'U' must have a
    /// 'vector_type' typedef.
    template<typename F, typename ...U>
    Array& apply(const U&... u) {
      if (!checksize(*this, u...)) throw std::out_of_range("size mismatch");
      for (idx_type n=0; n<v.size(); ++n) {
        v[n]->template apply<F, typename U::vector_type...>(u.getcol(n)...);
      }
      return *this;
    } 

    template<class F, typename U>
    Array& apply_scalar_post(U u) {
      for (idx_type n=0; n<v.size(); ++n) {
        v[n]->template apply_scalar_post<F, U>(u);
      }
      return *this;
    } 

    template<typename F, typename U>
    Array& apply(const U& u) {
      if (u.size() == 1) {
        return apply_scalar_post<F, typename U::value_type>(u[0]);
      }      
      for (idx_type n=0; n<v.size(); ++n) {
        v[n]->template apply<F, typename U::vector_type>(u.getcol(n));
      }
      return *this;
    } 

    template<class F, typename U, typename A>
    Array& apply_attrib(const Array<U>& u, const A& a) {
      if (u.size() == 1) {
        return apply_attrib_scalar_post<F,U>(u[0], a);
      }      
      // checkdim will check all the dimensions:
      if (!checkdims(dim, u.dim, std::max(dim.size(), u.size()) + 1)) {
        throw(std::out_of_range("in apply, array dimension mismatch"));
      }
      for (idx_type n=0; n<v.size(); ++n) {
        for (idx_type i=0; i<v[n]->size(); ++i) {
          setv_checkbefore(*v[n], i, F()((*v[n])[i], (*u.v[n])[i], a));
        }
      }
      return *this;
    } 

    template<class F, typename U, typename A>
    Array& apply_attrib_scalar_post(U u, const A& a) {
      for (idx_type n=0; n<v.size(); ++n) {   
        for (idx_type i=0; i<v[n]->size(); ++i) {
          setv_checkbefore(*v[n], i, F()((*v[n])[i], u, a));
        }
      }
      return *this;
    } 

    
    template <typename AO=O>
    Array& sort() {
      std::for_each(v.begin(), v.end(), [](const auto& e){ e->template sort<AO>(); });
      return *this;
    } 

    template <typename AO=O>
    Array& sort(idx_type col) {
      if (col >= v.size()) {
        throw std::range_error("sort column out of bounds");
      }
      v[col]->template sort<AO>();
      return *this;
    } 

    template<class UnaryFunction>
    UnaryFunction for_each(UnaryFunction f) const {
      for (auto& vi : v) {
        f = std::for_each(vi->begin(), vi->end(), f);
      }
      return f;
    }
    
    // We would want to enforce that U has to be numeric and
    // convertible to an idx_type.    
    template<typename U, typename AO=O>
    Array<U> sort_idx(idx_type base) const {
      vector<unique_ptr<Vector<U>>> idxv;
      for (idx_type j=0; j<v.size(); ++j) {
        idxv.emplace_back(make_unique<Vector<U>>(v[j]->template sort_idx<U, AO>(base)));
      }
      return Array<U>(dim, idxv, names); // this makes a copy of idxv, so not very good...
    } 

    // ----------------------- what should really be a private section:
    Vector<idx_type> dim;
    // we use unique_ptr below to avoid the copying of 'Vector' or
    // 'Dname' that would otherwise occur on 'vector' resize:
    vector<unique_ptr<Vector<T,O>>> v;        //! v is a set of cols, v[0], v[1] are the cols
    vector<unique_ptr<Dname>> names;

    std::unique_ptr<AllocFactory> allocf;
  };                            
  // ---------------------- end struct Array --------------------


  /// Vectorize to a std::vector of type R.
  template<typename R, typename T>
  std::vector<R> stdvector(const Array<T> u) {
    std::vector<R> r;
    for (auto& e: u.v) {
      r.insert(r.end(), e->begin(), e->end());
    }
    return r;
  }

  /// Vectorize to a arr::Vector of type R.
  template<typename R, typename T>
  Vector<R> arrvector(const Array<T> u) {
    Vector<R> r;
    for (auto& e: u.v) {
      r.insert(r.end(), e->begin(), e->end());
    }
    return r;
  }

  /// provide a const version of the above that works only with arrays that have 1 col LLL
  

  /// Return an array that is an n x 1 vector.
  template<typename T>
  Array<T> vectorize(const Array<T>& u) {
    idx_type n = u.v.size() * u.dim[0];
    Array<T> r(rsv, Vector<idx_type>{n});
    for (auto& e: u.v) {
      r.v[0]->insert(r.v[0]->end(), e->begin(), e->end());
    }
    return r;
  }


  template<>
  inline Array<arr::zstring> vectorize(const Array<arr::zstring>& u) {
    idx_type n = u.v.size() * u.dim[0];
    Array<zstring> r(rsv, {n});
    for (auto& e: u.v) {
      for (auto iter=e->begin(); iter!=e->end(); ++iter) { 
        r.v[0]->push_back(*iter);
      }
    }
    return r;
  }


  template<typename T, typename O=std::less<T>>
    Array<T,O> transpose(const Array<T,O>& t) {
    if (t.dim.size() == 1) {
      return Array<T,O>({1, t.dim[0]}, static_cast<Vector<T,O>>(*t.v[0]), {{}, t.names[0]->names});
    } 
    else if (t.dim.size() == 2) {
      Array<T,O> a(noinit_tag,
                   {t.getdim(1), t.getdim(0)}, {{t.getnames(1).names}, t.getnames(0).names});
      for (idx_type j=0; j<a.getdim(1); ++j)
        for (idx_type k=0; k<a.getdim(0); ++k)
          setv_checkbefore(a.getcol(j), k, t.getcol(k)[j]);
      return a;
    }
    else {
      throw range_error("argument is not a matrix");
    }
  }


  // swap dimensions:

  // resize? LLL 


  // array ops -----------------------------------------------

  template<typename T, typename R, typename OT=std::less<T>, typename OR=std::less<R>>
  Array<R,OR> applyf(const Array<T,OT>& t, function<R(T)> f) {
    auto ret = Array<R,OR>(rsv, t.dim);
    for (idx_type j=0; j<t.names.size(); ++j) { 
      ret.names[j] = make_unique<Dname>(*t.names[j]);
    }
    for (idx_type n=0; n<t.v.size(); ++n) {
      for (idx_type i=0; i<t.v[n]->size(); ++i) {
        ret.v[n]->push_back(f((*t.v[n])[i]));
      }
    }
    return ret;
  } 


  template<typename T, typename U, typename R, class F, 
           typename OU=std::less<U>, typename OR=std::less<R>>
  Array<R,OR> apply_scalar(T t, const Array<U,OU>& u) {
    Array<R,OR> ret(rsv, u.dim);
    for (idx_type j=0; j<u.names.size(); ++j) { 
      ret.names[j] = make_unique<Dname>(*u.names[j]);
    }
    for (idx_type n=0; n<u.v.size(); ++n) {   
      for (idx_type i=0; i<u.v[n]->size(); ++i) {
        ret.v[n]->push_back(F()(t, (*u.v[n])[i]));
      }
    }
    return ret;
  } 

  template<typename U, typename T, typename R, class F, 
           typename OU=std::less<U>, typename OR=std::less<R>>
    Array<R,OR> apply_scalar(const Array<U,OU>& u, T t) {
    Array<R,OR> ret(rsv, u.dim);
    for (idx_type j=0; j<u.names.size(); ++j) { 
      ret.names[j] = make_unique<Dname>(*u.names[j]);
    }
    for (idx_type n=0; n<u.v.size(); ++n) {   
      for (idx_type i=0; i<u.v[n]->size(); ++i) {
        ret.v[n]->push_back(F()((*u.v[n])[i], t));
      }
    }
    return ret;
  } 


  template <typename T, typename O>
  void setv_checkbefore(Array<T,O>& a, idx_type i, T t) {
    if (!a.dim.size())  throw range_error("subscript out of bounds");
    idx_type col = i / a.dim[0];
    idx_type off = i % a.dim[0];
    if (col >= a.ncols())  throw range_error("subscript out of bounds");
    setv_checkbefore(*a.v[col], off, t);
  }
  
  template <typename T, typename O>
  void setv(Array<T,O>& a, idx_type i, T t) {
    if (!a.dim.size())  throw range_error("subscript out of bounds");
    idx_type col = i / a.dim[0];
    idx_type off = i % a.dim[0];
    if (col >= a.ncols())  throw range_error("subscript out of bounds");
    setv(*a.v[col], off, t);    
  }


  /// Apply F to all parameters. Assumes same size, but not
  /// necessarily same dimensions. We pay a performance penalty for
  /// the generality.
  template<typename F, typename R, typename O, typename U1, typename ...U>
  Array<R,O> apply(const U1 u1, const U&... u) {
    if (!checksize(u1, u...)) throw std::out_of_range("size mismatch");
    
    auto ret = Array<R,O>(rsv, u1.dim);
    idx_type ii = 0;
    for (idx_type j=0; j<u1.names.size(); ++j) { 
      ret.names[j] = make_unique<Dname>(*u1.names[j]);
    }
    for (idx_type n=0; n<u1.v.size(); ++n) {
      for (idx_type j=0; j<u1.v[n]->size(); ++j) {
        ret.v[n]->push_back(F()((*u1.v[n])[j], u[ii]...));
        ++ii;
      }
    }
    return ret;
  } 

  template<typename F, typename R, typename O, typename U1, typename ...U>
  Array<R,O> apply_samesize(const U1 u1, const U&... u) {
    if (!checkdims(u1, u...)) throw std::out_of_range("size mismatch");
    auto ret = Array<R,O>(rsv, u1.dim);
    for (idx_type j=0; j<u1.names.size(); ++j) { 
      ret.names[j] = make_unique<Dname>(*u1.names[j]);
    }
    for (idx_type n=0; n<u1.v.size(); ++n) {
      for (idx_type i=0; i<u1.v[n]->size(); ++i) {
        ret.v[n]->push_back(F()((u1->getcol[n])[i], (u->getcol[n])[i])...);
      }
    }
    return ret;
  } 

  template<typename T, typename U, typename R, class F, 
           typename OT=std::less<T>, typename OU=std::less<U>, typename OR=std::less<R>>
  Array<R,OR> apply(const Array<T,OT>& t, const Array<U,OU>& u) {
    if (t.size() == 1) {
      return apply_scalar<T,U,R,F,OU,OR>(t[0], u);
    }
    else if (u.size() == 1) {
      return apply_scalar<T,U,R,F,OT,OR>(t, u[0]);
    }
    // if we are not in the case where one or the other array sizes is
    // 1, then they have to be of the same dimensions; we could allow
    // same size at the detriment of efficiency...
    if (t.dim != u.dim) {       // LLL allow differences with trailing 1's
      throw std::range_error("incompatible array sizes");
    }
    auto ret = Array<R>(rsv, u.dim);
    // create variadic find names function LLL
    for (idx_type j=0; j<u.names.size(); ++j) { 
      ret.names[j] = make_unique<Dname>(t.hasNames(j) ? *t.names[j] : *u.names[j]);
    }
    for (idx_type n=0; n<u.v.size(); ++n) {
      for (idx_type i=0; i<u.v[n]->size(); ++i) {
        ret.v[n]->push_back(F()((*t.v[n])[i], (*u.v[n])[i]));
      }
    }
    return ret;
  } 

  template<typename T, typename U, typename R, class F, typename A, 
           typename OU=std::less<U>, typename OR=std::less<R>>
    Array<R,OR> apply_attrib_scalar_post(T t, const Array<U,OU>& u, const A& a) {
    Array<R,OR> ret(rsv, u.dim);
    for (idx_type j=0; j<u.names.size(); ++j) { 
      ret.names[j] = make_unique<Dname>(*u.names[j]);
    }
    for (idx_type n=0; n<u.v.size(); ++n) {   
      for (idx_type i=0; i<u.v[n]->size(); ++i) {
        ret.v[n]->push_back(F()(t, (*u.v[n])[i], a));
      }
    }
    return ret;
  } 

  template<typename U, typename T, typename R, class F, typename A, 
           typename OU=std::less<U>, typename OR=std::less<R>>
    Array<R,OR> apply_attrib_scalar_pre(const Array<U,OU>& u, T t, const A& a) {
    Array<R,OR> ret(rsv, u.dim);
    for (idx_type j=0; j<u.names.size(); ++j) { 
      ret.names[j] = make_unique<Dname>(*u.names[j]);
    }
    for (idx_type n=0; n<u.v.size(); ++n) {   
      for (idx_type i=0; i<u.v[n]->size(); ++i) {
        ret.v[n]->push_back(F()((*u.v[n])[i], t, a));
      }
    }
    return ret;
  } 


  template<typename T, typename U, typename R, class F, typename A, 
           typename OT=std::less<T>, typename OR=std::less<R>, typename OU=std::less<U>>
    Array<R,OR> apply_attrib(const Array<T,OT>& t, const Array<U,OU>& u, const A& a) {
    if (t.size() == 1) {
      return apply_attrib_scalar_post<T,U,R,F,A,OU,OR>(t[0], u, a);
    }
    else if (u.size() == 1) {
      return apply_attrib_scalar_pre<T,U,R,F,A,OT,OR>(t, u[0], a);
    }
    // if we are not in the case where one or the other array sizes is
    // 1, then they have to be of the same dimensions; we could allow
    // same size at the detriment of efficiency...
    if (t.dim != u.dim) {       // LLL allow differences with trailing 1's
      throw std::range_error("incompatible array sizes");
    }
    auto ret = Array<R,OR>(rsv, u.dim);
    for (idx_type j=0; j<u.names.size(); ++j) { 
      ret.names[j] = make_unique<Dname>(t.hasNames(j) ? *t.names[j] : *u.names[j]);
    }
    for (idx_type n=0; n<u.v.size(); ++n) {
      for (idx_type i=0; i<u.v[n]->size(); ++i) {
        ret.v[n]->push_back(F()((*t.v[n])[i], (*u.v[n])[i], a));
      }
    }
    return ret;
  } 

  template<typename T, typename O=std::less<T>>
    Array<T,O> maxcol(const Array<T,O>& t) {
    // take care of usual too small matrices etc. LLL
    Vector<idx_type> dim{1};
    dim.insert(dim.end(), t.dim.begin()+1, t.dim.end());
    Array<T,O> ret(rsv, dim);
    for (idx_type n=0; n<t.v.size(); ++n) {
      T maxelt = (*t.v[n])[0];
      for (auto elt : *t.v[n]) {
        if (elt > maxelt) maxelt = elt;
      }
      ret.v[n]->push_back(maxelt);
    }
    return ret;
  }

  template<typename T, typename R, template <class> class F, typename O=std::less<T>>
    R cumul(const Array<T,O>& t, const R& init) {
    auto res = init; 
    for (idx_type n=0; n<t.v.size(); ++n) {
      for (idx_type i=0; i<t.v[n]->size(); ++i) {
        res = F<R>()(res, (*t.v[n])[i]);
      }
    }
    return res;
  } 

  /// among other things, this allows to implement an 'all' and 'any' function.
  // template<typename T, typename R>
  template<typename T, typename R, template <class> class F, typename O=std::less<T>>
    R cumul_until(const Array<T,O>& t, const R& init, const R& until) {
    auto res = init; 
    for (idx_type n=0; n<t.v.size(); ++n) {
      for (idx_type i=0; i<t.v[n]->size(); ++i) {
        res = F<R>()(res, (*t.v[n])[i]);
        if (res == until) {
          return res;
        }
      }
    }
    return res;
  } 

  // helper function to convert unidimentional arrays to standard vectors
  template<typename T, typename O=std::less<T>>
  inline vector<T> toStdVector(const Array<T,O>& a) {
    if (!a.isVector()) {
      throw out_of_range("multidimentional array cannot be converted to 'vector'");
    }
    return a.v[0];
  }

  /// Construct an array moving a vector.
  template<typename T, typename O=std::less<T>>
  inline Array<T,O> array_from_vector(Vector<T,O>&& data)
  {
    if (!data.size()) {
      throw std::out_of_range("array_from_vector: null array creation not supported");
    }
    Array<T> a(rsv, {data.size()});
    a.v[0]->swap(data);
    return a;
  }

  
}

#endif
