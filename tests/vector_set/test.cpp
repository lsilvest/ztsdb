// -*- compile-command: "make -k -j -O test" -*-

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
#include "vector.hpp"
#include "base_types.hpp"
#include "vector_set.hpp"
#include "timezone/vector_set_time.hpp"
#include "timezone/ztime.hpp"
#include "timezone/ztime_vector.hpp"
#include "misc.hpp"
#include "../utils_dtime.hpp"


using namespace arr;
using namespace std::string_literals;

tz::Zones tzones("/usr/share/zoneinfo");


// union ------------------------------------
TEST(union_sorted) {
  Vector<double> v1{1,2,3,4};
  Vector<double> v2{1,2,5,6};
  Vector<double> exp{1,2,3,4,5,6};
  auto res = arr::_union(v1, v2);
  ASSERT_TRUE(res == exp);
}
TEST(union_unsorted_v1) {
  Vector<double> v1{9,8,2,3,4};
  Vector<double> v2{1,2,5,6};
  Vector<double> exp{1,2,3,4,5,6,8,9};
  auto res = arr::_union(v1, v2);
  ASSERT_TRUE(res == exp);
}
TEST(union_unsorted_v2) {
  Vector<double> v1{1,2,5,6};
  Vector<double> v2{9,8,2,3,4};
  Vector<double> exp{1,2,3,4,5,6,8,9};
  auto res = arr::_union(v1, v2);
  ASSERT_TRUE(res == exp);
}
TEST(union_unsorted) {
  Vector<double> v1{6,1,5,2};
  Vector<double> v2{9,8,2,3,4};
  Vector<double> exp{1,2,3,4,5,6,8,9};
  auto res = arr::_union(v1, v2);
  ASSERT_TRUE(res == exp);
}
TEST(union_unsorted_default) {
  Vector<double> v1{6,1,5,2};
  Vector<double> v2{9,8,2,3,4};
  Vector<double> exp{1,2,3,4,5,6,8,9};
  auto res = arr::_union(v1, v2);
  ASSERT_TRUE(res == exp);
}
TEST(union_same) {
  Vector<double> v1{6,1,5,2};
  Vector<double> exp{1,2,5,6};
  auto res = arr::_union(v1, v1);
  ASSERT_TRUE(res == exp);
}
TEST(union_duplicates) {
  Vector<double> v1{2,2,2,2,2,2};
  Vector<double> v2{1,1,1,1};
  Vector<double> exp{1,2};
  auto res = arr::_union(v1, v2);
  for (size_t i=0; i<res.size(); ++i) {
    std::cout << res[i] << std::endl;
  }
  ASSERT_TRUE(res == exp);
}
TEST(union_duplicates_all) {
  Vector<double> v1{1,1,1};
  Vector<double> v2{1,1,1,1,1};
  Vector<double> exp{1};
  auto res = arr::_union(v1, v2);
  for (size_t i=0; i<res.size(); ++i) {
    std::cout << res[i] << std::endl;
  }
  ASSERT_TRUE(res == exp);
}
TEST(union_v1_empty) {
  Vector<double> v1{};
  Vector<double> v2{5,4,3,2,1};
  Vector<double> exp{1,2,3,4,5};  
  auto res = arr::_union(v1, v2);
  ASSERT_TRUE(res == exp);
}
TEST(union_v2_empty) {
  Vector<double> v1{1,2,3,4};
  Vector<double> v2{};
  auto res = arr::_union(v1, v2);
  ASSERT_TRUE(res == v1);
}
TEST(union_v1_v2_empty) {
  Vector<double> v1{};
  Vector<double> v2{};
  Vector<double> exp{};
  auto res = arr::_union(v1, v2);
  ASSERT_TRUE(res == exp);
}
TEST(union_unsorted_string) {
  Vector<zstring> v1{"b","c","a","e"};
  Vector<zstring> v2{"a","f","e","d"};
  Vector<zstring> exp{"a","b","c","d","e","f"};
  auto res = arr::_union(v1, v2);
  ASSERT_TRUE(res == exp);
}
// intersect ------------------------------------
TEST(intersect_sorted) {
  Vector<double> v1{1,2,3,4};
  Vector<double> v2{1,2,5,6};
  Vector<double> exp{1,2};
  auto res = arr::intersect(v1, v2);
  for (size_t i=0; i<res.size(); ++i) {
    std::cout << res[i] << std::endl;
  }
  ASSERT_TRUE(res == exp);
}
TEST(intersect_unsorted_v1) {
  Vector<double> v1{9,8,2,3,4};
  Vector<double> v2{1,2,5,6};
  Vector<double> exp{2};
  auto res = arr::intersect(v1, v2);
  ASSERT_TRUE(res == exp);
}
TEST(intersect_unsorted_v2) {
  Vector<double> v1{1,2,3,5,6};
  Vector<double> v2{9,8,2,4,3};
  Vector<double> exp{2,3};
  auto res = arr::intersect(v1, v2);
  ASSERT_TRUE(res == exp);
}
TEST(intersect_unsorted) {
  Vector<double> v1{6,1,9,5,2};
  Vector<double> v2{9,8,2,3,4};
  Vector<double> exp{2,9};
  auto res = arr::intersect(v1, v2);
  ASSERT_TRUE(res == exp);
}
TEST(intersect_unsorted_default) {
  Vector<double> v1{6,1,5,2};
  Vector<double> v2{9,8,2,3,4};
  Vector<double> exp{2};
  auto res = arr::intersect(v1, v2);
  ASSERT_TRUE(res == exp);
}
TEST(intersect_same) {
  Vector<double> v1{6,1,5,2};
  Vector<double> exp{1,2,5,6};
  auto res = arr::intersect(v1, v1);
  ASSERT_TRUE(res == exp);
}
TEST(intersect_duplicates) {
  Vector<double> v1{2,2,1,1,2,2,2,2};
  Vector<double> v2{1,1,1,1};
  Vector<double> exp{1};
  auto res = arr::intersect(v1, v2);
  for (size_t i=0; i<res.size(); ++i) {
    std::cout << res[i] << std::endl;
  }
  ASSERT_TRUE(res == exp);
}
TEST(intersect_v1_empty) {
  Vector<double> v1{};
  Vector<double> v2{5,4,3,2,1};
  Vector<double> exp{};
  auto res = arr::intersect(v1, v2);
  ASSERT_TRUE(res == exp);
}
TEST(intersect_v2_empty) {
  Vector<double> v1{1,2,3,4};
  Vector<double> v2{};
  Vector<double> exp{};
  auto res = arr::intersect(v1, v2);
  ASSERT_TRUE(res == exp);
}
TEST(intersect_v1_v2_empty) {
  Vector<double> v1{};
  Vector<double> v2{};
  Vector<double> exp{};
  auto res = arr::intersect(v1, v2);
  ASSERT_TRUE(res == exp);
}
TEST(intersect_empty) {
  Vector<double> v1{6,7,8,9};
  Vector<double> v2{5,4,3,2,1};
  Vector<double> exp{};
  auto res = arr::intersect(v1, v2);
  ASSERT_TRUE(res == exp);
}
TEST(intersect_unsorted_string) {
  Vector<zstring> v1{"b","c","a","e"};
  Vector<zstring> v2{"a","f","e","d"};
  Vector<zstring> exp{"a","e"};
  auto res = arr::intersect(v1, v2);
  ASSERT_TRUE(res == exp);
}
// setdiff ------------------------------------
TEST(setdiff_sorted) {
  Vector<double> v1{1,2,3,4};
  Vector<double> v2{2,3};
  Vector<double> exp{1,4};
  auto res = arr::setdiff(v1, v2);
  for (size_t i=0; i<res.size(); ++i) {
    std::cout << res[i] << std::endl;
  }
  ASSERT_TRUE(res == exp);
}
// intersect_idx --------------------------------
TEST(intersect_idx_sorted) {
  Vector<double> v1{0,1,2,3,4};
  Vector<double> v2{-1,1,2,5,6};
  auto exp = std::make_pair(Vector<double>{2,3}, Vector<double>{2,3});
  auto res = arr::intersect_idx<double,double,double>(v1, v2);
  for (size_t i=0; i<res.first.size(); ++i) {
    std::cout << res.first[i] << std::endl;
  }
  ASSERT_TRUE(res == exp);
}
// union_idx --------------------------------

TEST(union_idx_sorted) {
  Vector<double> v1{1,2,3,4};
  Vector<double> v2{1,2,5,6};
  auto exp = std::make_pair(Vector<double>{1,2,3,4,NANF<double>::f(),NANF<double>::f()}, 
                            Vector<double>{1,2,NANF<double>::f(),NANF<double>::f(),5,6});
  auto res = arr::union_idx<double,double,double,NANF>(v1, v2);
  ASSERT_TRUE(exp == exp);
}
// setdiff_idx --------------------------------
TEST(setdiff_idx_sorted) {
  Vector<double> v1{1,2,3,4};
  Vector<double> v2{2,3,5};
  auto exp = std::make_pair(Vector<double>{1,4}, Vector<double>{3});
  auto res = arr::setdiff_idx<double,double,double>(v1, v2);
  ASSERT_TRUE(res == exp);
}
// intersect time/intervals -----------------------------
TEST(intersect_time_interval) {
  Vector<Global::dtime> v1{mkt(1), mkt(3), mkt(6), mkt(7), mkt(8), mkt(12)};
  Vector<tz::interval> v2{mki(2,6), mki(8,11)};
  Vector<Global::dtime> exp{mkt(3), mkt(6), mkt(8)};
  auto res = arr::intersect<Global::dtime,tz::interval>(v1, v2);
  for (size_t i=0; i<res.size(); ++i) {
    std::cout << res[i].time_since_epoch().count() << std::endl;
  }
  ASSERT_TRUE(res == exp);
}
// intersect interval/interval -------------------------
TEST(intersect_interval) {
  auto dt1 = tz::interval_from_string("|+2015-03-09 06:38:01 America/New_York "
                                      "-> 2015-03-10 06:38:01 America/New_York+|", tzones);
  auto dt2 = tz::interval_from_string("|+2015-03-09 06:38:02 America/New_York "
                                      "-> 2015-03-10 06:38:00 America/New_York+|", tzones);
  Vector<tz::interval> v1{dt1};
  Vector<tz::interval> v2{dt2};
  auto res = arr::intersect<tz::interval,tz::interval>(v1, v2);
  ASSERT_TRUE(res == v2);
}
// union interval/interval  -------------------------
// 0 1 2 3 4 5 6 7 8 9 1011
//   --------- ----- 
//     -----     ----- ----
TEST(union_interval) {
  Vector<tz::interval> v1{mki(1,3), mki(3,5), mki(6,8)};
  Vector<tz::interval> v2{mki(2,4), mki(7,9), mki(10,11)};
  Vector<tz::interval> exp{mki(1,5), mki(6,9), mki(10,11)};
  auto res = arr::_union<tz::interval,tz::interval>(v1, v2);
  ASSERT_TRUE(res == exp);
}
// 0 1 2 3 4 5 6 7 8 9 1011
//   ------------- -------- 
//     --- ---       ---
TEST(union_interval_multisubset) {
  Vector<tz::interval> v1{mki(1,7), mki(8,11)};
  Vector<tz::interval> v2{mki(2,3), mki(4,5), mki(9,10)};
  auto res = arr::_union<tz::interval,tz::interval>(v1, v2);
  ASSERT_TRUE(res == v1);
}
// setdiff interval/interval -------------------------
//    0 1 2 3 4 5 6 7 8 9 1011
// v1   --------- -----   ----
// v2     -----     ----- 
// res  --     --   ---   ----
const auto t = true;
const auto f = false;
TEST(setdiff_interval_1) {
  Vector<tz::interval> v1{mki(1,3), mki(3,5), mki(6,8), mki(10,11)};
  Vector<tz::interval> v2{mki(2,4), mki(7,9)};
  Vector<tz::interval> exp{mki(1,2,f,t), mki(4,5,t,f), mki(6,7,f,t), mki(10,11)};
  auto res = arr::setdiff<tz::interval,tz::interval>(v1, v2);
  const auto tzstring = "America/New_York";
  for (size_t i=0; i<res.size(); ++i) {
    std::cout << tz::to_string(exp[i], "", tzones.find(tzstring), tzstring) << std::endl;
    std::cout << tz::to_string(res[i], "", tzones.find(tzstring), tzstring) << std::endl;
  }
  ASSERT_TRUE(res == exp);
}
//    0 1 2 3 4 5 6 7 8 9 1011
// v1   ----
// v2 --------
TEST(setdiff_interval_2) {
  Vector<tz::interval> v1{mki(1,2)};
  Vector<tz::interval> v2{mki(0,3)};
  Vector<tz::interval> exp{};
  auto res = arr::setdiff<tz::interval,tz::interval>(v1, v2);
  ASSERT_TRUE(res == exp);
}

int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
