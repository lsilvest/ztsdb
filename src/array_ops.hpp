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


#ifndef ARRAY_OPS
#define ARRAY_OPS


#include "valuevector.hpp"
#include "functional"
#include "array.hpp"
#include "globals.hpp"


namespace arr {

  /// window (or rolling window) function to compute the mean on that
  /// sliding window. This function does not use Kahan summation.
  template<typename T>
  Array<T>& rollmean_inplace(Array<T>& a, idx_type window, idx_type nbvalid) 
  {
    size_t ringsz = 1; 
    while (ringsz < window) ringsz <<= 1;
    vector<T> ring(ringsz);

    for (idx_type c=0; c<a.v.size(); c++) {
      idx_type nbv = 0;
      T mean = 0;               // R is the result type (because
                                // e.g. for T as an int we probably
                                // still want R as double)
      for (idx_type r=0; r<a.dim[0]; ++r) {
        T outvalue = ring[r & (ringsz-1)];
        if (!std::isnan((*a.v[c])[r])) {
          mean += (*a.v[c])[r];
          ring[r & (ringsz - 1)] = (*a.v[c])[r]; 
          ++nbv;
        }
        if (r >= window && !std::isnan(outvalue)) {
          mean -= outvalue;
          --nbv;
        }
        if (nbv >= nbvalid) {
          setv(a.getcol(c), r, mean / window);
        } else {
          setv(a.getcol(c), r, Global::ZNAN);
        }
      } 
    }
    return a;    
  }



  /// window (or rolling window) function to compute the minimum on
  /// a sliding window. In addition to the window size 'window', an
  /// additional parameter 'nbvalid' specifies how many observations
  /// are needed to consider the calculation valid. If the number of
  /// missing (i.e. NaN) values is larger than 'window' - 'nbvalid'
  /// then NaN will be returned for that window.
  template<typename T>
  Array<T>& rollmin_inplace(Array<T>& a, idx_type window, idx_type nbvalid) {
    auto minima = new std::pair<T, idx_type>[a.dim[0]]; // the minimum and its position

    for (idx_type c=0; c<a.v.size(); c++) {
      idx_type nbv = 0, front = 0, back = 0;
      for (idx_type r=0; r<a.dim[0]; ++r) {
        if (!std::isnan((*a.v[c])[r])) {
          // pop at the back the values that are greater than what we
          // are adding because they cannot be minima anymore:
          while (back > front && minima[back-1].first >= (*a.v[c])[r]) --back;
          minima[back].first  = (*a.v[c])[r];
          minima[back].second = r;
          ++back;
          ++nbv;
        }
        if (r >= window && !std::isnan((*a.v[c])[r-window])) {
          --nbv;
        }
        if (nbv >= nbvalid) {
          if (r >= window) {
            // pop at the front until we find a minimum that is in our
            // window:
            while (minima[front].second <= r-window) ++front;
          }
          setv(a.getcol(c), r, minima[front].first); // that's our minimum!
        } else {
          setv(a.getcol(c), r, Global::ZNAN);
        }
      } 
    }
    delete[] minima;           // this is OK as the above can't throw
    return a;
  }



  /// window (or rolling window) function to compute the maximum on
  /// that sliding window. In addition to the window size 'window', an
  /// additional parameter 'nbvalid' specifies how many observations
  /// are needed to consider the calculation valid. If the number of
  /// missing (i.e. NA) values is larger than 'window' - 'nbvalid'
  /// then NA will be returned for that window.
  template<typename T>
  Array<T>& rollmax_inplace(Array<T>& a, idx_type window, idx_type nbvalid) {
    auto maxima = new std::pair<T, idx_type>[a.dim[0]]; // the minimum and its position

    for (idx_type c=0; c<a.v.size(); c++) {
      idx_type nbv = 0, front = 0, back = 0;
      for (idx_type r=0; r<a.dim[0]; ++r) {
        if (!std::isnan((*a.v[c])[r])) {
          // pop at the back the values that are smaller than what we
          // are adding because they cannot be maxima anymore:
          while (back > front && maxima[back-1].first <= (*a.v[c])[r]) --back;
          maxima[back].first  = (*a.v[c])[r];
          maxima[back].second = r;
          ++back;
          ++nbv;
        }
        if (r >= window && !std::isnan((*a.v[c])[r-window])) {
          --nbv;
        }
        if (nbv >= nbvalid) {
          if (r >= window) {
            // pop at the front until we find a maximum that is in our
            // window:
            while (maxima[front].second <= r-window) ++front;
          }
          setv(a.getcol(c), r, maxima[front].first); // that's our minimum!
        } else {
          setv(a.getcol(c), r, Global::ZNAN);
        }
      } 
    }
    delete[] maxima;           // this is OK as the above can't throw
    return a;
  }

  /// window (or rolling window) function to compute the variance on
  /// that sliding window. The variance formula used is the unbiased
  /// estimator with divisor = n-1. This function does not use Kahan
  /// summation.
  //
  // 1/(n-1) S (x - mx)^2 = 1/(n-1) S (x2 - 2mxx + mx^2) = 1/(n-1) (S
  // x2 - 2 mx S x + n mx^2) = 1/(n-1) (S x2 - 2/n (S x)^2 + 1/n (S
  // x)^2) = 1/(n-1) (S x2 - 1/n (S x)^2)
  template<typename T>
  Array<T>& rollvar_inplace(Array<T>& a, idx_type window, idx_type nbvalid) {
    size_t ringsz = 1; 
    while (ringsz < window) ringsz <<= 1;
    vector<T> ring(ringsz);
    vector<T> ring2(ringsz);


    for (idx_type c=0; c<a.v.size(); c++) {
      idx_type nbv = 0;
      T sum = 0, sum2 = 0;
      
      for (idx_type r=0; r<a.dim[0]; ++r) {
        T outvalue  = ring[r & (ringsz-1)];
        T outvalue2 = ring2[r & (ringsz-1)];
        if (!std::isnan((*a.v[c])[r])) {
          sum  += (*a.v[c])[r];
          ring[r & (ringsz - 1)] = (*a.v[c])[r]; 
          sum2 += (*a.v[c])[r] * (*a.v[c])[r];
          ring2[r & (ringsz - 1)] = (*a.v[c])[r] * (*a.v[c])[r]; 
          ++nbv;
        }
        if (r >= window && !std::isnan(outvalue)) {
          sum  -= outvalue;
          sum2 -= outvalue2;
          --nbv;
        }
        if (nbv >= nbvalid) {
          setv(a.getcol(c), r, (sum2 - sum*sum / nbv) / (nbv - 1));
          if ((*a.v[c])[r] < 0) {     // possible because of small rounding errors
            setv(a.getcol(c), r, 0.0);
          }
        } else {
          setv(a.getcol(c), r, Global::ZNAN);
        }
      } 
    }
    return a;    
  }

  
  template<typename T>
  Array<T>& locf_inplace(Array<T>& a, ssize_t n) {
    for (idx_type c=0; c<a.v.size(); c++) {
      auto last_idx = -1L;
      for (idx_type r=1; r<a.dim[0]; ++r) {
        if (std::isnan((*a.v[c])[r])) {
          if (last_idx < 0) {
            last_idx = r - 1;
          }
          setv(a.getcol(c), r, (r - last_idx <= static_cast<idx_type>(n)) ? (*a.v[c])[r-1] : Global::ZNAN);
        } else {
          last_idx = -1;
        }
      } 
    }
    return a;
  }

  
  template<typename T>
  Array<T>& move_inplace(Array<T>& a, ssize_t n) {
    if (n > 0 ) {
      for (idx_type c=0; c<a.v.size(); c++) {
        for (idx_type r=a.dim[0]-1; r >= static_cast<idx_type>(n); --r) {
          setv(a.getcol(c), r, (*a.v[c])[r - n]);
        }
        for (idx_type r=0; r<static_cast<idx_type>(n); ++r) {
          setv(a.getcol(c), r, Global::ZNAN);
        }
      }
    } 
    else {
      for (idx_type c=0; c<a.v.size(); c++) {
        for (idx_type r=0; r<a.dim[0] + n; ++r) {
          setv(a.getcol(c), r, (*a.v[c])[r - n]);
        }
        for (idx_type r=a.dim[0]+n; r<a.dim[0]; ++r) {
          setv(a.getcol(c), r, Global::ZNAN);
        }
      }
    }
    return a;
  }


  template<typename T>
  Array<T>& rotate_inplace(Array<T>& a, ssize_t n) {
    if (n > 0 ) {
      vector<T> v(n);
      for (idx_type c=0; c<a.v.size(); c++) {
        for (idx_type r=a.dim[0]-n; r<a.dim[0]; ++r) {
          v[r-(a.dim[0]-n)] = (*a.v[c])[r];
        }
        for (idx_type r=a.dim[0]-1; r >= static_cast<idx_type>(n); --r) {
          setv(a.getcol(c), r, (*a.v[c])[r - n]);
        }
        for (idx_type r=0; r<static_cast<idx_type>(n); ++r) {
          setv(a.getcol(c), r, v[r]);
        }
      }
    } 
    else {
      vector<T> v(-n);
      for (idx_type c=0; c<a.v.size(); c++) {
        for (idx_type r=0; r<static_cast<idx_type>(-n); ++r) {
          v[r] = (*a.v[c])[r];
        }
        for (idx_type r=0; r<a.dim[0] + n; ++r) {
          setv(a.getcol(c), r, (*a.v[c])[r - n]);
        }
        for (idx_type r=a.dim[0]+n; r<a.dim[0]; ++r) {
          setv(a.getcol(c), r, v[r - (a.dim[0]+n)]);
        }
      }
    }
    return a;
  }

  
  template<typename T>
  Array<T>& diff_inplace(Array<T>& a, ssize_t n) {
    if (n > 0 ) {
      for (idx_type c=0; c<a.v.size(); c++) {
        for (idx_type r=a.dim[0]-1; r >= static_cast<idx_type>(n); --r) {
          setv(a.getcol(c), r, (*a.v[c])[r] - (*a.v[c])[r - n]);
        }
        for (idx_type r=0; r<static_cast<idx_type>(n); ++r) {
          setv(a.getcol(c), r, Global::ZNAN);
        }
      }
    } 
    else {
      for (idx_type c=0; c<a.v.size(); c++) {
        for (idx_type r=0; r<a.dim[0] + n; ++r) {
          setv(a.getcol(c), r, (*a.v[c])[r] - (*a.v[c])[r - n]);
        }
        for (idx_type r=a.dim[0]+n; r<a.dim[0]; ++r) {
          setv(a.getcol(c), r, Global::ZNAN);
        }
      }
    }
    return a;
  }




  /// window (or rolling window) function to compute the covariance on
  /// a sliding window. In addition to the window size 'window', an
  /// additional parameter 'nbvalid' specifies how many observations
  /// are needed to consider the calculation valid. If the number of
  /// missing (i.e. NaN) values is larger than 'window' - 'nbvalid'
  /// then NaN will be returned for that window.
  //
  // S (x-xm)(y-ym) = S (xy - y xm - x ym + xm ym) = 
  // S xy - xm S y - ym S x + n xm ym = 
  // S xy - (S x S y) / n - (S x S y) / n + n S x S y / n^2 = 
  // S xy - S x S y / n
  //
  // so we can use the unbiased estimator:
  // cov(x, y) = 1/(n-1) (S xy - S x S y / n)
  template<typename T>
  Array<T> rollcov(const Array<T>& x, const Array<T>& y, idx_type window, idx_type nbvalid) {
    size_t ringsz = 1; 
    while (ringsz < window) ringsz <<= 1;
    vector<T> ring_x(ringsz);
    vector<T> ring_y(ringsz);
    vector<T> ring_xy(ringsz);

    if (!(x.getdim().size() && y.getdim().size() && (x.getdim(0) == y.getdim(0)))) {
      throw std::range_error("invalid dimensions");
    }
    
    Array<T> z(rsv, Vector<idx_type>{0, x.ncols(), y.ncols()});

    // obviously we could improve the efficiency and calculate the cov
    // only for the triangle of columns...
    const idx_type nrows = y.getdim(0);
    idx_type zcol = 0;
    for (idx_type colx=0; colx < x.ncols(); ++colx) {
      for (idx_type coly=0; coly < y.ncols(); ++coly) {
        idx_type nbv = 0;
        T sum_x = 0, sum_y = 0, sum_xy = 0;

        for (idx_type r=0; r<nrows; ++r) { // rows
          T outvalue_x  = ring_x[r & (ringsz-1)];
          T outvalue_y  = ring_y[r & (ringsz-1)];
          T outvalue_xy = ring_xy[r & (ringsz-1)];
          if (!std::isnan(x.getcol(colx)[r]) && !std::isnan(y.getcol(coly)[r])) {
            sum_x  += x.getcol(colx)[r];
            ring_x[r & (ringsz - 1)] = x.getcol(colx)[r]; 
            sum_y  += y.getcol(coly)[r];
            ring_y[r & (ringsz - 1)] = y.getcol(coly)[r]; 
            sum_xy += x.getcol(colx)[r] * y.getcol(coly)[r];
            ring_xy[r & (ringsz - 1)] = x.getcol(colx)[r] * y.getcol(coly)[r];
            ++nbv;
          }
          if (r >= window && !std::isnan(outvalue_x) && !std::isnan(outvalue_y)) {
            sum_x  -= outvalue_x;
            sum_y  -= outvalue_y;
            sum_xy -= outvalue_xy;
            --nbv;
          }
          if (nbv >= nbvalid) {
            z.getcol(zcol).push_back((sum_xy - sum_x*sum_y / nbv) / (nbv - 1));
          } else {
            z.getcol(zcol).push_back(Global::ZNAN);
          }
        }
        ++zcol;
      }
      
      // build names 
      z.names[0]->resize(nrows);
      setv(z.dim, 0, nrows);
    }
    return z;    
  }


  template<typename T, typename F>
  Array<T>& cumul_inplace(Array<T>& a, bool rev) {
    if (!rev) {
      for (idx_type c=0; c < a.ncols(); c++) {
        for (idx_type r=1; r < a.getcol(c).size(); ++r) {
          setv(a.getcol(c), r, F()(a.getcol(c)[r], a.getcol(c)[r-1]));
        }
      }
    }
    else {
      for (idx_type c=0; c < a.ncols(); c++) {
        for (idx_type r=a.getcol(c).size()-1; r >= 1 ; --r) {
          setv(a.getcol(c), r-1, F()(a.getcol(c)[r-1], a.getcol(c)[r]));
        }
      }      
    }
    return a;
  } 

  template<typename T>
  Array<T>& rev_inplace(Array<T>& a) {
    for (idx_type c=0; c < a.ncols(); c++) {
      auto iter_start = a.getcol(c).begin();
      auto iter_end   = a.getcol(c).end();
      for (arr::idx_type j=0; j<a.getcol(c).size() / 2; j++) {
        std::iter_swap(iter_start++, --iter_end);
      }
    }
    return a;
  }
    
} // namespace arr


#endif
