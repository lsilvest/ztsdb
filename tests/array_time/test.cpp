// -*- compile-command: "make -j -k test" -*-

// Copyright (C) 2015 Leonardo Silvestri
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


#include <numeric>
#include <cmath>
#include <crpcut.hpp>
#include <ftw.h>
#include "array.hpp"
// #include "display.hpp"
#include "index.hpp"
#include "array_ops.hpp"
#include "../utils_dtime.hpp"
#include "../utils.hpp"
#include "timezone/ztime.hpp"

tz::Zones tzones("/usr/share/zoneinfo");


// helper function for the creation of indices
namespace arr {

  template<typename T>
  static Index make_NullIndex(idx_type d, const Array<T>& t) {
    return NullIndex{t.dim[d]};
  }

  static Index make_IntIndex(const arr::Vector<size_t>& v) {
    return IntIndex(v);
  }

  // static Index make_BoolIndex(const Vector<bool>& v) {
  //   return BoolIndex{v};
  // }

  template<typename T>
  static Index make_NameIndex(idx_type d, const vector<string>& v, const Array<T>& t) {
    return NameIndex{v, *t.names[d]};
  }
}


// 1D slices
TEST(array_4_subset_dtime) {
  auto a = arr::Array<Global::dtime>({4}, {mkt(1),mkt(2),mkt(3),mkt(4)});
  auto b = arr::Array<Global::dtime>({2}, {mkt(1),mkt(3)});
  
  auto tidx = arr::Vector<Global::dtime>{mkt(1),mkt(3)}; ;
  auto idx = vector<arr::Index>{arr::DtimeIndex{tidx, a.getcol(0)}};
                          
  auto res = a(idx);
  ASSERT_TRUE(res==b);
}
TEST(array_4_subset_interval) {
  auto a = arr::Array<Global::dtime>({4}, {mkt(1),mkt(2),mkt(3),mkt(4)});
  auto b = arr::Array<Global::dtime>({2}, {mkt(2),mkt(3)});
  
  auto tidx = arr::Vector<tz::interval>{mki(2,3)}; ;
  auto idx = vector<arr::Index>{arr::IntervalIndex{tidx, a.getcol(0)}};
                          
  auto res = a(idx);
  std::cout << res.size() << std::endl;
  ASSERT_TRUE(res==b);
}

// 2D slices
// array of dtime:
TEST(array_2x2_subset_dtime) {
  auto a = arr::Array<Global::dtime>({2,2}, {mkt(1),mkt(2),mkt(3),mkt(4)}, 
    {{"un","deux"}, {"one", "two"}});
  auto b = arr::Array<Global::dtime>({2}, {mkt(1),mkt(3)}, {{"one", "two"}});
  ASSERT_TRUE((a(vector<arr::Index>{arr::make_IntIndex({0L}), arr::make_NullIndex(1,a)})==b));
}


int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
