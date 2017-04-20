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


#ifndef ZTIME_VECTOR_HPP
#define ZTIME_VECTOR_HPP



#include "../vector.hpp"
#include "../globals.hpp"
#include "../misc.hpp"
#include "../pseudovector.hpp"
//#include "zone.hpp"
#include "interval.hpp"
#include "ztime.hpp"


namespace ztsdb {

  template <typename T>
  arr::Vector<T> seq(T from, T to, tz::period by, const tz::Zone& z);
  template <typename T>
  arr::Vector<T> seq(T from, tz::period by, size_t n, const tz::Zone& z);

}


namespace arr {                 // should be in tz? LLL

  // align -----------------------------------

  /// This class allows the templatization of the align functions by
  /// providing binary arithmetic operators for operators that require
  /// a timezone.

  template <typename T, typename U>
  struct PseudoVectorTz {
    PseudoVectorTz(const Vector<U>& v_p, const tz::Zone& tz_p) : 
      v(v_p), scalar(v.size() == 1), first_elt(v[0]), tz(tz_p) { }
    inline const U operator[](size_t i) const { return scalar ? first_elt : v[i]; }

    inline const T plus(T t, U u) const { return tz::plus(t, u, tz); }

  private:
    const Vector<U>& v;
    const bool scalar;
    const U& first_elt;
    const tz::Zone& tz;
  };


  template <typename I, typename NANF, 
            typename DS, typename DE>
  arr::Vector<I> align_idx(const arr::Vector<Global::dtime>& x, 
                           const arr::Vector<Global::dtime>& y, 
                           const DS& start, 
                           const DE& end) 
  {
    arr::Vector<I> res;
    size_t ix = 0, iy = 0;

    // for each point in y, we try to find a matching point or set of
    // points in x:
    for (iy=0; iy<y.size(); iy++) {
      auto ystart = start.plus(y[iy], start[iy]);
      auto yend   = end.plus(y[iy], end[iy]);
      
      // advance until we have a point in x that is in the interval
      // defined around yi:
      while (ix <= x.size() && x[ix] < ystart) ++ix;
      if (ix >= x.size() || x[ix] > yend) {
        res.push_back(NANF::f());
        continue;
      }

      // find the closest point in the interval:
      while (ix+1 < x.size() && x[ix+1] <= yend && tz::abs(x[ix] - y[iy]) > tz::abs(x[ix+1] - y[iy])) ++ix;
      res.push_back(ix + 1);     // +1 because of R numbering start convention
    }
    return res;
  }
  
  
  /// This is the same algorithm as 'align_idx', except that instead
  /// of returning a vector of indices, the vector 'ydata' is modified
  /// in place with the data pulled from x.
  template <typename T, typename NANF, 
            typename DS, typename DE>
  void align_closest(const arr::Vector<Global::dtime>& x, 
                     const arr::Vector<Global::dtime>& y, 
                     const arr::Vector<T>& xdata, 
                     arr::Vector<T>& ydata, 
                     const DS& start, 
                     const DE& end) 
  {
    size_t ix = 0, iy = 0;

    if (xdata.size() != x.size()) throw std::out_of_range("'xdata' must have same size as 'x'");   

    // for each point in y, we try to find a matching point or set of
    // points in x:
    for (iy=0; iy<y.size(); iy++) {
      auto ystart = start.plus(y[iy], start[iy]);
      auto yend   = end.plus(y[iy], end[iy]);
    
      // advance until we have a point in x that is in the interval
      // defined around yi:
      while (ix < x.size() && x[ix] < ystart) ++ix;
      if (ix >= x.size() || x[ix] > yend) {
        ydata.push_back(NANF::f());
        continue;
      }

      // find the closest point in the interval:
      while (ix+1 < x.size() && x[ix+1] <= yend && tz::abs(x[ix] - y[iy]) > tz::abs(x[ix+1] - y[iy]))
        ++ix;
      ydata.push_back(xdata[ix]);
    }
  }


  template <typename T, typename F,
            typename DS, typename DE>
  void align_func(const arr::Vector<Global::dtime>& x, 
                  const arr::Vector<Global::dtime>& y, 
                  const arr::Vector<T>& xdata, 
                  arr::Vector<T>& ydata, 
                  const DS& start, 
                  const DE& end) 
  {
    size_t ix = 0, iy = 0;

    if (xdata.size() != x.size()) throw std::out_of_range("'xdata' must have same size as 'x'");   

    // for each point in y, we try to find a matching point or set of
    // points in x:
    for (iy=0; iy<y.size(); iy++) {
      auto ystart = start.plus(y[iy], start[iy]);
      auto yend   = end.plus(y[iy], end[iy]);
    
      // advance until we have a point in x that is in the interval
      // defined around yi:
      auto iter = std::lower_bound(x.begin() + ix, x.end(), ystart);
      ix = iter - x.begin();
      
      if (ix >= x.size() || x[ix] >= yend) {
        ydata.push_back(F::f(xdata.end(), xdata.end())); // empty interval
        continue;
      }
      typename arr::Vector<T>::const_iterator istart(xdata, ix);
      auto first_ix = ix;

      // find the last point in the interval:
      iter = std::lower_bound(x.begin() + ix, x.end(), yend);
      ix = iter - x.begin();
      while (ix < x.size() && x[ix] < yend) ++ix;
      typename arr::Vector<T>::const_iterator iend(xdata, ix);

      ydata.push_back(F::f(istart, iend));

      // reset ix to the first ix found, because the intervals
      // specified could overlap:
      ix = first_ix;
    }
  }

  template <typename T, typename F>
  void op_zts(const arr::Vector<Global::dtime>& x, 
              const arr::Vector<Global::dtime>& y, 
              const arr::Vector<T>& xdata, 
              arr::Vector<T>& ydata) 
  {
    size_t ix = 0;

    if (xdata.size() != x.size()) throw std::out_of_range("'xdata' must have same size as 'x'");   

    // for each point in x, we try to find a matching point or set of
    // points in y:
    auto from_yiter = y.begin();
    for (ix=0; ix<x.size(); ix++) {
      auto to_yiter = std::lower_bound(from_yiter, y.end(), x[ix]);
      if (to_yiter == y.end()) continue;

      auto iy_s = from_yiter-y.begin();
      auto iy_e = to_yiter-y.begin();
      F::f(xdata[ix], ydata.begin() + iy_s, ydata.begin() + iy_e);
      
      from_yiter = to_yiter;
    }
  }

} // namespace arr


#endif
