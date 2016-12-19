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


#include "ztime_vector.hpp"
#include "ztime.hpp"
#include "../hinnant_date/date.h"


using namespace std::chrono;
using namespace std::literals;


// seq -----------------------


template <typename T>
arr::Vector<T> ztsdb::seq(T from, T to, tz::period by, const tz::Zone& z) {
  arr::Vector<T> r{from};
  auto diff = to-from;
  auto pos = diff >= 0s;
  auto dist = tz::abs(diff);
  auto olddist = dist;
  for (;;) {
    auto next = tz::plus(r.back(), by, z);
    if (pos ? next > to : next < to) break;
    r.push_back(next);
    olddist = dist;
    dist = tz::abs(to-next);
    if (dist >= olddist) {
      throw std::range_error("incorrect specification for 'to'/'by'");
    }
  }
  return r;
}


template <typename T>
arr::Vector<T> ztsdb::seq(T from, tz::period by, size_t n, const tz::Zone& z) {
  arr::Vector<T> r{from};
  for (size_t i=1; i<n; ++i) {
    r.push_back(tz::plus(r[i-1], by, z));
  }
  return r;
}


// explicit instantiation:
template arr::Vector<Global::dtime>
ztsdb::seq(Global::dtime from, Global::dtime to, tz::period by, const tz::Zone& z);
template arr::Vector<tz::interval>
ztsdb::seq(tz::interval from, tz::interval to, tz::period by, const tz::Zone& z);

template arr::Vector<Global::dtime>
ztsdb::seq(Global::dtime from, tz::period by, size_t n, const tz::Zone& z);
template arr::Vector<tz::interval>
ztsdb::seq(tz::interval from, tz::period by, size_t n, const tz::Zone& z);
