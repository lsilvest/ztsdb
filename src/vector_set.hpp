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


#ifndef VECTOR_SET_HPP
#define VECTOR_SET_HPP


#include <algorithm>
#include <utility>
#include "vector.hpp"


/// 
namespace arr { 

  template <typename T, typename U>
  Vector<T> intersect(const Vector<T>& v1, const Vector<U>& v2);

  template <typename T, typename U, typename I>
  std::pair<Vector<I>, Vector<I>> intersect_idx(const Vector<T>& v1, const Vector<U>& v2);

  template <typename T, typename U>
  Vector<T> _union(const Vector<T>& v1, const Vector<U>& v2);

  template <typename T, typename U, typename I>
  std::pair<Vector<I>, Vector<I>> union_idx(const Vector<T>& v1, const Vector<U>& v2);


  template <typename T, typename U>
  Vector<T> setdiff(const Vector<T>& v1, const Vector<U>& v2);


  template <typename T, typename U, typename I>
  std::pair<Vector<I>, Vector<I>> setdiff_idx(const Vector<T>& v1, const Vector<U>& v2);
 
}


// ----------------------------------------------------------------------------
// Implementation.

namespace arr {

  template <typename T, typename U, template <typename, typename> class F>
  static Vector<T> conditional_sort_helper(const Vector<T>& v1, const Vector<U>& v2) 
  {
    if (!v1.isOrdered() && !v2.isOrdered()) {
      auto s1 = v1;
      std::sort(s1.begin(), s1.end()); // OOO LLL use a vector sort 
      auto s2 = v2;
      std::sort(s2.begin(), s2.end());
      return F<T,U>::f(s1, s2);
    }
    if (!v1.isOrdered()) {
      auto s1 = v1;
      std::sort(s1.begin(), s1.end());
      return F<T,U>::f(s1, v2);
    }
    if (!v2.isOrdered()) {
      auto s2 = v2;
      std::sort(s2.begin(), s2.end());
      return F<T,U>::f(v1, s2);
    }
    return F<T,U>::f(v1, v2);
  }


  template <typename T, typename U, typename I, template <typename, typename, typename> class F>
  static std::pair<Vector<I>, Vector<I>> 
    conditional_sort_idx_helper(const Vector<T>& v1, const Vector<U>& v2) 
  {
    // we need the indices to be correct, so if we sorted we need to translate the result
    if (!v1.isOrdered() && !v2.isOrdered()) {
      auto s1 = v1;
      std::sort(s1.begin(), s1.end());
      auto s2 = v2;
      std::sort(s2.begin(), s2.end());
      return F<T,U,I>::f(s1, s2);
    }
    if (!v1.isOrdered()) {
      auto s1 = v1;
      std::sort(s1.begin(), s1.end());
      return F<T,U,I>::f(s1, v2);
    }
    if (!v2.isOrdered()) {
      auto s2 = v2;
      std::sort(s2.begin(), s2.end());
      return F<T,U,I>::f(v1, s2);
    }
    return F<T,U,I>::f(v1, v2);
  }


  template <typename T, 
            typename U, 
            typename I, 
            typename NANF, 
            template <typename, typename, typename, typename> class F>
  static std::pair<Vector<I>, Vector<I>> 
    conditional_sort_idx_nan_helper(const Vector<T>& v1, const Vector<U>& v2) 
  {
    // we need the indices to be correct, so if we sorted we need to translate the result
    if (!v1.isOrdered() && !v2.isOrdered()) {
      auto s1 = v1;
      std::sort(s1.begin(), s1.end());
      auto s2 = v2;
      std::sort(s2.begin(), s2.end());
      return F<T,U,I,NANF>::f(s1, s2);
    }
    if (!v1.isOrdered()) {
      auto s1 = v1;
      std::sort(s1.begin(), s1.end());
      return F<T,U,I,NANF>::f(s1, v2);
    }
    if (!v2.isOrdered()) {
      auto s2 = v2;
      std::sort(s2.begin(), s2.end());
      return F<T,U,I,NANF>::f(v1, s2);
    }
    return F<T,U,I,NANF>::f(v1, v2);
  }



  // --------------------------------------
  // intersects ---------------------------

  template <typename T, typename U>
  struct intersect_helper {
    static Vector<T> f(const Vector<T>& v1, const Vector<U>& v2) 
    {
      Vector<T> res;
      size_t i1 = 0, i2 = 0;
      while (i1 < v1.size() && i2 < v2.size()) {
        if (v1[i1] < v2[i2]) {
          ++i1;
        } else if (v1[i1] > v2[i2]) {
          ++i2;
        } else { 
          if (res.size()==0 || v1[i1] != res.back()) {
            res.push_back(v1[i1]);
          }      
          ++i1;
          //++i2; this is correct for T==U, but not for example when
          //T==time and U==interval
        }
      }
      return res;
    }
  };


  template <typename T, typename U>
  Vector<T> intersect(const Vector<T>& v1, const Vector<U>& v2) 
  {
    return conditional_sort_helper<T, U, intersect_helper>(v1, v2);
  }


  template <typename T, typename U, typename I>
  struct intersect_idx_helper {
    static std::pair<Vector<I>, Vector<I>> f(const Vector<T>& v1, const Vector<U>& v2) 
    {
      std::pair<Vector<I>, Vector<I>> res;
      size_t i1 = 0, i2 = 0;
      while (i1 < v1.size() && i2 < v2.size()) {
        if (v1[i1] < v2[i2]) {
          ++i1;
        } else if (v1[i1] > v2[i2]) {
          ++i2;
        } else { 
          if (v1.size()==0 || v1[i1] != v1[i1-1]) {
            res.first.push_back(i1+1);
            res.second.push_back(i2+1);
          }      
          ++i1;
          //++i2; this is correct for T==U, but not for example when
          //T==time and U==interval
        }
      }
      return res;
    }
  };

  template <typename T, typename U, typename I>
  std::pair<Vector<I>, Vector<I>> intersect_idx(const Vector<T>& v1, const Vector<U>& v2)
  {
    return conditional_sort_idx_helper<T, U, I, intersect_idx_helper>(v1, v2);
  }


  // --------------------------------------
  // unions -------------------------------

  template <typename T, typename U>
  struct union_helper {
    static Vector<T> f(const Vector<T>& v1, const Vector<U>& v2)
    {
      Vector<T> res;
      size_t i1 = 0, i2 = 0;
      while (i1 < v1.size() && i2 < v2.size()) {
        if (v1[i1] < v2[i2]) {
          if (res.size()==0 || v1[i1] != res.back()) {
            res.push_back(v1[i1]);
          }
          ++i1;
        } else if (v1[i1] > v2[i2]) {
          if (res.size()==0 || v2[i2] != res.back()) {
            res.push_back(v2[i2]);
          }
          ++i2;
        } else {
          if (res.size()==0 || v1[i1] != res.back()) {
            res.push_back(v1[i1]);
          }
          ++i1;
          ++i2;
        }
      }

      while (i1 < v1.size()) {
        if (i1==0 || v1[i1] != v1[i1-1]) {
          res.push_back(v1[i1]);
        }
        ++i1;
      }   
      while (i2 < v2.size()) {
        if (i2==0 || v2[i2] != v2[i2-1]) {
          res.push_back(v2[i2]);
        }
        ++i2;
      }   

      return res;
    }
  };

  template <typename T, typename U>
  Vector<T> _union(const Vector<T>& v1, const Vector<U>& v2) 
  {
    return conditional_sort_helper<T, U, union_helper>(v1, v2);
  }


  template <typename T, typename U, typename I, typename NANF>
  struct union_idx_helper {
    static std::pair<Vector<I>, Vector<I>> f(const Vector<T>& v1, const Vector<U>& v2)
    {
      std::pair<Vector<I>, Vector<I>> res;
      size_t i1 = 0, i2 = 0;
      while (i1 < v1.size() && i2 < v2.size()) {
        if (v1[i1] < v2[i2]) {
          if (v1.size()==0 || v1[i1] != v1[i1-1]) {
            res.first.push_back(i1+1);
            res.second.push_back(NANF::f());
          }
          ++i1;
        } else if (v1[i1] > v2[i2]) {
          if (v2.size()==0 || v2[i2] != v1[i2-1]) {
            res.first.push_back(NANF::f());
            res.second.push_back(i2+1);
          }
          ++i2;
        } else {
          if (v1.size()==0 || v1[i1] != v1[i1-1]) {
            res.first.push_back(i1+1);
            res.second.push_back(i2+1);
          }
          ++i1;
          ++i2;
        }
      }

      while (i1 < v1.size()) {
        if (i1==0 || v1[i1] != v1[i1-1]) {
          res.first.push_back(i1+1);
          res.second.push_back(NANF::f());
        }
        ++i1;
      }   
      while (i2 < v2.size()) {
        if (i2==0 || v2[i2] != v2[i2-1]) {
          res.first.push_back(NANF::f());
          res.second.push_back(i2+1);
        }
        ++i2;
      }   

      return res;
    }
  };

  template <typename T, typename U, typename I, template <typename> class NANF>
    std::pair<Vector<I>, Vector<I>> union_idx(const Vector<T>& v1, 
                                              const Vector<U>& v2) 
  {
    return conditional_sort_idx_nan_helper<T, U, I, NANF<I>, union_idx_helper>(v1, v2);
  }


  // --------------------------------------
  // setdiffs -------------------------------

  template <typename T, typename U>
  struct setdiff_helper {
    static Vector<T> f(const Vector<T>& v1, 
                       const Vector<U>& v2) 
    {
      Vector<T> res;
      size_t i1 = 0, i2 = 0;
      while (i1 < v1.size() && i2 < v2.size()) {
        if (v1[i1] < v2[i2]) {
          res.push_back(v1[i1++]);
        } else if (v1[i1] > v2[i2]) {
          ++i2;
        } else { 
          ++i1;
          //++i2; this is correct for T==U, but not for example when
          //T==time and U==interval
         }
      }

      // pick up elts left in v1:
      while (i1 < v1.size()) {
        res.push_back(v1[i1++]);
      }

      return res;
    }
  };

  template <typename T, typename U>
  Vector<T> setdiff(const Vector<T>& v1, const Vector<U>& v2) 
  {
    return conditional_sort_helper<T, U, setdiff_helper>(v1, v2);
  }


  template <typename T, typename U, typename I>
  struct setdiff_idx_helper {
    static std::pair<Vector<I>, Vector<I>> f(const Vector<T>& v1, const Vector<U>& v2) 
    {
      std::pair<Vector<I>, Vector<I>> res;
      size_t i1 = 0, i2 = 0;
      while (i1 < v1.size() && i2 < v2.size()) {
        if (v1[i1] < v2[i2]) {
          res.first.push_back(i1+1);
          ++i1;
        } else if (v1[i1] > v2[i2]) {
          res.second.push_back(i2+1);
          ++i2;
        } else { 
          ++i1;
          ++i2;
        }
      }

      // pick up elts left in v1:
      while (i1 < v1.size()) {
        res.first.push_back(i1+1);
        ++i1;
      }
      // pick up elts left in v2:
      while (i2 < v2.size()) {
        res.second.push_back(i2+1);
        ++i2;
      }

      return res;
    }
  };

  template <typename T, typename U, typename I>
  std::pair<Vector<I>, Vector<I>> setdiff_idx(const Vector<T>& v1, const Vector<U>& v2)
  {
    return conditional_sort_idx_helper<T, U, I, setdiff_idx_helper>(v1, v2);
  }


}

#endif
