// -*- compile-command: "make -k -j test" -*-

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


#include <cmath>
#include <crpcut.hpp>
#include <algorithm>
#include "vector_set.hpp"
#include "base_types.hpp"
#include "timezone/vector_set_time.hpp"
#include "timezone/ztime.hpp"
#include "timezone/ztime_vector.hpp"
#include "misc.hpp"
#include "../utils_dtime.hpp"
#include "zts.hpp"
#include "index.hpp"
#include "period.hpp"
#include "align_funcs.hpp"


using namespace arr;
using namespace std::string_literals;

tz::Zones tzones("/usr/share/zoneinfo");


// vector -----------------------------
TEST(align_idx) {
  Vector<Global::dtime> x{mkt(1), mkt(3), mkt(6), mkt(7), mkt(8),  mkt(12)};
  Vector<Global::dtime> y{mkt(2), mkt(4), mkt(6), mkt(8), mkt(10), mkt(11)};
  Vector<double> exp{1,2,3,5,TNAN,TNAN};
  using namespace std::literals;
  Vector<Global::duration> start{-10ms};
  Vector<Global::duration> end{0s};
  auto res = arr::align_idx<double,NANF<double>,
                            arr::PseudoVector<Global::dtime, Global::duration>,
                            arr::PseudoVector<Global::dtime, Global::duration>>
    (x, y, start, end);
  std::cout << res.size() << std::endl;
  for (size_t i=0; i<res.size(); ++i) {
    std::cout << res[i] << std::endl;
  }
  ASSERT_TRUE(res == exp);
}
TEST(align_closest_double) {
  Vector<Global::dtime> x{mkt(1), mkt(3), mkt(6), mkt(7), mkt(8),  mkt(12)};
  Vector<Global::dtime> y{mkt(2), mkt(4), mkt(6), mkt(8), mkt(10), mkt(11)};
  Vector<double> xdata{10,20,30,40,50,60};
  Vector<double> ydata(rsv, y.size());
  Vector<double> exp{10,20,30,50,TNAN,TNAN};
  Vector<Global::duration> start{-10ms};
  Vector<Global::duration> end{0s};
  arr::align_closest<double, NANF<double>,
                     arr::PseudoVector<Global::dtime, Global::duration>,
                     arr::PseudoVector<Global::dtime, Global::duration>>
    (x, y, xdata, ydata, start, end);
  std::cout << ydata.size() << std::endl;
  for (size_t i=0; i<ydata.size(); ++i) {
    std::cout << ydata[i] << ", " << exp[i] << std::endl;
  }
  ASSERT_TRUE(ydata == exp);
}
TEST(align_func_max) {
  Vector<Global::dtime> x{mkt(0), mkt(1), mkt(2), mkt(3), mkt(4), mkt(5), mkt(6), 
                          mkt(7), mkt(8), mkt(9), mkt(10), mkt(11), mkt(12)};
  Vector<Global::dtime> y{mkt(3), mkt(6), mkt(9), mkt(12)};
  Vector<double> xdata{1,7,2, 3,4,5, 9,1,1, 2,2,1, 0};
  Vector<double> ydata(rsv, y.size());
  Vector<double> exp{7,5,9,2};
  Vector<Global::duration> start{-30ms};
  Vector<Global::duration> end{0s};
  using FIter = Vector<double>::const_iterator;
  arr::align_func<double, ztsdb::max_element<FIter,double>, 
                  arr::PseudoVector<Global::dtime, Global::duration>,
                  arr::PseudoVector<Global::dtime, Global::duration>>
    (x, y, xdata, ydata, start, end);
  std::cout << ydata.size() << std::endl;
  for (size_t i=0; i<ydata.size(); ++i) {
    std::cout << ydata[i] << std::endl;
  }
  ASSERT_TRUE(ydata == exp);
}
TEST(align_func_min) {
  Vector<Global::dtime> x{mkt(0), mkt(1), mkt(2), mkt(3), mkt(4), mkt(5), mkt(6), 
                          mkt(7), mkt(8), mkt(9), mkt(10), mkt(11), mkt(12)};
  Vector<Global::dtime> y{mkt(3), mkt(6), mkt(9), mkt(12)};
  Vector<double> xdata{1,7,2, 3,4,5, 9,1,1, 2,2,1, 0};
  Vector<double> ydata(rsv, y.size());
  Vector<double> exp{1,3,1,1};
  Vector<Global::duration> start{-30ms};
  Vector<Global::duration> end{0s};
  using FIter = Vector<double>::const_iterator;
  arr::align_func<double, ztsdb::min_element<FIter,double>, 
                  arr::PseudoVector<Global::dtime, Global::duration>,
                  arr::PseudoVector<Global::dtime, Global::duration>>
    (x, y, xdata, ydata, start, end);
  std::cout << ydata.size() << std::endl;
  for (size_t i=0; i<ydata.size(); ++i) {
    std::cout << ydata[i] << std::endl;
  }
  ASSERT_TRUE(ydata == exp);
}
TEST(align_func_mean) {
  Vector<Global::dtime> x{mkt(0), mkt(1), mkt(2), mkt(3), mkt(4), mkt(5), mkt(6), 
                          mkt(7), mkt(8), mkt(9), mkt(10), mkt(11), mkt(12)};
  Vector<Global::dtime> y{mkt(3), mkt(6), mkt(9), mkt(12)};
  Vector<double> xdata{1,7,2, 3,4,5, 9,1,1, 2,2,1, 0};
  Vector<double> ydata(rsv, y.size());
  Vector<double> exp{10/3.0,4,11/3.0,5/3.0};
  Vector<Global::duration> start{-30ms};
  Vector<Global::duration> end{0s};
  using FIter = Vector<double>::const_iterator;
  arr::align_func<double, ztsdb::mean_element<FIter,double>, 
                  arr::PseudoVector<Global::dtime, Global::duration>,
                  arr::PseudoVector<Global::dtime, Global::duration>>
    (x, y, xdata, ydata, start, end);
  for (unsigned i=0; i<ydata.size(); ++i) {
    std::cout << ydata[i] << " = " << exp[i] << std::endl;
  }
  ASSERT_TRUE(ydata == exp);
}
TEST(align_func_count) {
  Vector<Global::dtime> x{mkt(0), mkt(1), mkt(2), mkt(3), mkt(4), mkt(5), mkt(6), 
                          mkt(7), mkt(8), mkt(9), mkt(10), mkt(11), mkt(12)};
  Vector<Global::dtime> y{mkt(3), mkt(6), mkt(9), mkt(12)};
  Vector<double> xdata{1,7,2, 3,4,5, 9,1,1, 2,2,1, 0};
  Vector<double> ydata(rsv, y.size());
  Vector<double> exp{3,3,3,3};
  Vector<Global::duration> start{-30ms};
  Vector<Global::duration> end{0s};
  using FIter = Vector<double>::const_iterator;
  arr::align_func<double, ztsdb::count_element<FIter,double>, 
                  arr::PseudoVector<Global::dtime, Global::duration>,
                  arr::PseudoVector<Global::dtime, Global::duration>>
    (x, y, xdata, ydata, start, end);
  std::cout << "ydata[0]: " << ydata[0] << ", ydata[1]: " << ydata[1] << std::endl;
  ASSERT_TRUE(ydata == exp);
}
TEST(align_func_median) {
  Vector<Global::dtime> x{mkt(0), mkt(1), mkt(2), mkt(3), mkt(4), mkt(5), mkt(6), 
                          mkt(7), mkt(8), mkt(9), mkt(10), mkt(11), mkt(12)};
  Vector<Global::dtime> y{mkt(3), mkt(6), mkt(9), mkt(12)};
  Vector<double> xdata{1,7,2, 3,4,5, 9,1,1, 2,2,1, 0};
  Vector<double> ydata(rsv, y.size());
  Vector<double> exp{2,4,1,2};
  Vector<Global::duration> start{-30ms};
  Vector<Global::duration> end{0s};
  using FIter = Vector<double>::const_iterator;
  arr::align_func<double, ztsdb::median_element<FIter,double>, 
                  arr::PseudoVector<Global::dtime, Global::duration>,
                  arr::PseudoVector<Global::dtime, Global::duration>>
    (x, y, xdata, ydata, start, end);
  ASSERT_TRUE(ydata == exp);
}
TEST(align_func_median_even) {
  Vector<Global::dtime> x{mkt(0), mkt(1), mkt(2), mkt(3), mkt(4), mkt(5), mkt(6), 
                          mkt(7), mkt(8), mkt(9), mkt(10), mkt(11), mkt(12)};
  Vector<Global::dtime> y{mkt(4), mkt(8), mkt(12)};
  Vector<double> xdata{1,7,2,2, 3,6,4,5, 9,1,1,1, 0};
  Vector<double> ydata(rsv, y.size());
  Vector<double> exp{2,4.5,1};
  Vector<Global::duration> start{-40ms};
  Vector<Global::duration> end{0s};
  using FIter = Vector<double>::const_iterator;
  arr::align_func<double, ztsdb::median_element<FIter,double>, 
                  arr::PseudoVector<Global::dtime, Global::duration>,
                  arr::PseudoVector<Global::dtime, Global::duration>>
    (x, y, xdata, ydata, start, end);
  ASSERT_TRUE(ydata == exp);
}

// zts --------------------------
TEST(zts_align_closest) {
  // make a zts based on x and a some random data
  Array<Global::dtime> idx(Vector<Global::dtime>{mkt(1), mkt(3), mkt(6), mkt(7), mkt(8), mkt(12)});
  Array<double> a(Vector<idx_type>{idx.size(), 2}, {1,2,3,4,5,6, 7,8,9,10,11,12});
  std::cout << "a.size(): " << a.size() << std::endl;
  zts z(idx, a);
  Vector<Global::duration> start{-10ms};
  Vector<Global::duration> end{0s};
  Array<Global::dtime> y({mkt(2), mkt(4), mkt(6), mkt(8), mkt(10), mkt(11)});
  auto res = arr::align_closest<arr::PseudoVector<Global::dtime, Global::duration>,
                  arr::PseudoVector<Global::dtime, Global::duration>>
    (z, y, start, end);
  arr::zts exp(y, Array<double>(Vector<idx_type>{idx.size(), 2}, 
    {1,2,3,5,TNAN,TNAN, 7,8,9,11,TNAN,TNAN}));
  ASSERT_TRUE(res == exp);
}

// zts --------------------------


int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
