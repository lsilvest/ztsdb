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


#ifndef VECTOR_SET_TIME_HPP
#define VECTOR_SET_TIME_HPP


#include "../vector_set.hpp"
#include "../globals.hpp"
#include "interval.hpp"


// ----------------------------------------------------------------------------
// Implementation.

namespace arr {

  /// intersect T=Global::dtime, U=Global::dtime doesn't need specialization.
  /// intersect T=Global::dtime, U=tz::interval doesn't need specialization.
  /// interval/interval is special:
  template <>
  struct intersect_helper<tz::interval, tz::interval> 
  {
    static Vector<tz::interval> f(const Vector<tz::interval>& v1, 
                                  const Vector<tz::interval>& v2) 
    {
      Vector<tz::interval> res;
      size_t i1 = 0, i2 = 0;
      while (i1 < v1.size() && i2 < v2.size()) {
        if (v1[i1].s >= v2[i2].e) {
          ++i1;
          continue;
        } else if (v2[i2].s >= v1[i1].e) {
          ++i1;
          continue;
        }

        auto v1_gt_v2 = tz::start_gt(v1[i1], v2[i2]);
        auto start = v1_gt_v2 ? v1[i1].s : v2[i2].s;
        auto sopen = v1_gt_v2 ? v1[i1].sopen : v2[i2].sopen;

        if (tz::end_lt(v1[i1], v2[i2])) {
          res.push_back(tz::interval(start, v1[i1].e, sopen, v1[i1].eopen));
          ++i1;
        } else {
          res.push_back(tz::interval(start, v2[i2].e, sopen, v2[i2].eopen));
          ++i2;
        }
      }
      return res;
    }
  };

  /// intersect_idx T=Global::dtime, U=Global::dtime doesn't need specialization.
  /// intersect_idx T=Global::dtime, U=tz::interval doesn't need specialization.
  /// intersect_idx interval/interval doesn't make sense.

  /// union T=Global::dtime, U=Global::dtime doesn't need specialization.
  /// union T=Global::dtime, U=tz::interval doesn't make sense.
  /// interval/interval is a special beast.
  template <>
  struct union_helper<tz::interval, tz::interval> {
    static Vector<tz::interval> f(const Vector<tz::interval>& v1, 
                                  const Vector<tz::interval>& v2) 
    {
      Vector<tz::interval> res;
      size_t i1 = 0, i2 = 0;
      if (v1.size() > 0 && v2.size() > 0) {
        auto v1_lt_v2 = tz::start_lt(v1[i1], v2[i2]);
        auto start = v1_lt_v2 ? v1[i1].s : v2[i2].s;
        auto sopen = v1_lt_v2 ? v1[i1].sopen : v2[i2].sopen;
  
        for (;;) {
          if (tz::end_ge_start(v1[i1], v2[i2]) && tz::end_le(v1[i1], v2[i2])) {
            // v1 |------------|         or     |--------|
            // v2      |------------|         |------------|
            if (++i1 >= v1.size()) {
              // v2 interval done, as there's no more v1 elts to overlap
              res.push_back(tz::interval(start, v2[i2].e, sopen, v2[i2].eopen));
              ++i2;
              break;
            }
          } else if (tz::end_ge_start(v2[i2], v1[i1]) && tz::end_le(v2[i2], v1[i1])) {
            // v1      |------------|   or    |------------|
            // v2 |------------|                |--------|
            if (++i2 >= v2.size()) {
              // v2 interval done, as there's no more v2 elts to overlap
              res.push_back(tz::interval(start, v1[i1].e, sopen, v1[i1].eopen));
              ++i1;
              break;
            }
          } else {
            // no interval overlap
            if (tz::end_lt(v1[i1], v2[i2])) {
              res.push_back(tz::interval(start, v1[i1].e, sopen, v1[i1].eopen));
              ++i1;
            } else {
              res.push_back(tz::interval(start, v2[i2].e, sopen, v2[i2].eopen));
              ++i2;
            }
            // set the start of the next interval:
            if (i1 < v1.size() && i2 < v2.size()) {
              start = std::min(v1[i1].s, v2[i2].s);
            } else {
              break;
            }
          }  
        }
      }
      // remaining non-overlapping intervals in v1:
      while (i1 < v1.size()) {
        res.push_back(v1[i1++]);
      }
      while (i2 < v2.size()) {
        res.push_back(v2[i2++]);
      }
      return res;
    }
  };


  /// union_idx T=Global::dtime, U=Global::dtime doesn't need specialization.
  /// union_idx T=Global::dtime, U=tz::interval doesn't make sense.
  /// union_idx interval/interval doesn't make sense.


  /// setdiff T=Global::dtime, U=Global::dtime doesn't need specialization.
  /// setdiff T=Global::dtime, U=tz::interval doesn't need specialization.
  /// interval/interval is special:
  template <>
  struct setdiff_helper<tz::interval, tz::interval> {
    static Vector<tz::interval> f(const Vector<tz::interval>& v1, 
                                  const Vector<tz::interval>& v2) 
    {
      Vector<tz::interval> res;
      size_t i1 = 0, i2 = 0;
      auto start = v1[i1].s;
      auto sopen = v1[i1].sopen;
      while (i1 < v1.size() && i2 < v2.size()) {
        if (tz::end_lt_start(v1[i1], v2[i2])) {
          // |-------------|
          //                 |------------|
          res.push_back(tz::interval(start, v1[i1].e, sopen, v1[i1].eopen));
          if (++i1 >= v1.size()) break;
          start = v1[i1].s;
          sopen = v1[i1].sopen;
        } else if (tz::start_lt(v2[i2].e, v2[i2].eopen, start, sopen)) {
          //                 |------------|
          // |-------------|
          ++i2;
        } else if (tz::start_lt(start, sopen, v2[i2].s, v2[i2].sopen)) {
          // |-------------|         or   |-------------| 
          //        |------------|           |-------|
          res.push_back(tz::interval(start, v2[i2].s, sopen, !v2[i2].sopen));
          if (tz::end_gt(v1[i1], v2[i2])) {
            // |-------------| 
            //    |-------|
            start = v2[i2].e;
            sopen = !v2[i2].eopen;
            ++i2;
          } else {
            // |-------------|
            //        |------------|
            if (++i1 >= v1.size()) break;
            start = v1[i1].s;
            sopen = v1[i1].sopen;
          }
        } else if (tz::start_ge(start, sopen, v2[i2].s, v2[i2].sopen) &&
                   tz::end_ge(v2[i2], v1[i1])) {
          //    |-------|
          // |-------------| 
          if (++i1 >= v1.size()) break;
          start = v1[i1].s;
          sopen = v1[i1].sopen;
        } else {
          //         |------------|
          //     |----------| 
          start = v2[i2].e;
          sopen = !v2[i2].eopen;
          ++i2;
        }

      }
      // remaining non-overlapping intervals in v1:
      if (i1 < v1.size()) {
        res.push_back(tz::interval(start, v1[i1].e, sopen, v1[i1].eopen));
        ++i1;
        while (i1 < v1.size()) {
          res.push_back(v1[i1++]);
        }        
      }
      return res;
    }
  };


  /// setdiff_idx T=Global::dtime, U=Global::dtime doesn't need specialization.
  /// setdiff_idx T=interval, U=interval doesn't make sense
  /// setdiff_idx T=Global::dtime, U=interval:
  template <typename I>
  struct setdiff_idx_helper<I, Global::dtime, tz::interval> {
    static Vector<I> f(const Vector<Global::dtime>& v1, 
                       const Vector<tz::interval>& v2) 
    {
      std::pair<Vector<I>, Vector<I>> res;
      size_t i1 = 0, i2 = 0;
      while (i1 < v1.size() && i2 < v2.size()) {
        if (v1[i1] < v2[i2]) {
          res.first.push_back(i1+1);
          ++i1;
        } else if (v1[i1] > v2[i2]) {
          ++i2;
        } else { 
          ++i1;
        }
      }

      // pick up elts left in v1:
      while (i1 < v1.size()) {
        res.first.push_back(i1+1);
        ++i1;
      }

      return res;
    }
  };

}

#endif
